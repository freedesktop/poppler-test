#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <poppler.h>
#include <glob.h>
#include "read-cache.h"
#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)


#define POPPLER_TEST_LOG_SUFFIX ".log"
#define POPPLER_TEST_PNG_SUFFIX "-out.png"
#define POPPLER_TEST_REF_SUFFIX "-ref.png"
#define POPPLER_TEST_DIFF_SUFFIX "-diff.png"


typedef enum poppler_test_status {
      POPPLER_TEST_SUCCESS = 0,
      POPPLER_TEST_FAILURE
} poppler_test_status_t;

int ilog10(int a)
{
  int l = 10;
  int log = 0;
  while (a >= l) {
    l *= 10;
    log++;
  }
  return log;
}

poppler_test_status_t gdk_pixbuf_compare(GdkPixbuf *pixbuf, char *page_name)
{
  char *png_name, *ref_name, *diff_name;
  char *srcdir;
  GError *error = NULL;
  int pixels_changed;
  const char *backend = poppler_get_backend() == POPPLER_BACKEND_SPLASH ? "splash" : "cairo";
   /* Get the strings ready that we'll need. */
  srcdir = getenv ("srcdir");
  if (!srcdir)
    srcdir = ".";
  asprintf (&png_name, "%s-%s%s", page_name, backend, POPPLER_TEST_PNG_SUFFIX);
  asprintf (&ref_name, "%s/%s-%s%s", srcdir, page_name, backend, POPPLER_TEST_REF_SUFFIX);
  asprintf (&diff_name, "%s-%s%s", page_name, backend, POPPLER_TEST_DIFF_SUFFIX);
#ifdef PNG_SAVE
  gdk_pixbuf_save (pixbuf, png_name, "png", &error, NULL);
  pixels_changed = image_diff (png_name, ref_name, diff_name);
#else
  int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

  int width = gdk_pixbuf_get_width (pixbuf);
  int height = gdk_pixbuf_get_height (pixbuf);

  int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  unsigned int *cairo_pixels = malloc(height * width * 4);
  unsigned char *pixels = gdk_pixbuf_get_pixels(pixbuf);
  unsigned char *pixels_p = pixels;

  int j,i;
  for (j=0; j<height; j++) {
    pixels = pixels_p + rowstride * j;
    for (i=0; i<width; i++) {
      cairo_pixels[i+width*j] = (0xff000000) | (pixels[0] << 16) | (pixels[1] << 8) | pixels[2] << 0;
      pixels += 3;
    }
  }

  pixels_changed = image_buf_diff (cairo_pixels, width, height, width*4, png_name, ref_name, diff_name);

  free (cairo_pixels);
#endif
  free (png_name);
  free (ref_name);
  free (diff_name);

  return pixels_changed ? POPPLER_TEST_FAILURE : POPPLER_TEST_SUCCESS;
}

poppler_test_status_t poppler_test_page(char *pdf_file, PopplerDocument *document, int page_index, int n_pages) {
  GdkPixbuf *pixbuf, *thumb;
  double width, height;
  PopplerPage *page;
  poppler_test_status_t ret;
  char *page_format;
  char *page_name;
  char *thumb_name;

  asprintf(&page_format, "%%s-%%0%dd", ilog10(n_pages)+1);
  asprintf(&page_name, page_format, pdf_file, page_index);
  asprintf(&thumb_name, "%s-thumb", page_name);

  page = poppler_document_get_page (document, page_index);
  if (!page)
    FAIL ("page not found");

  poppler_page_get_size (page, &width, &height);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
  gdk_pixbuf_fill (pixbuf, 0x00106000);
  poppler_page_render_to_pixbuf (page, 0, 0, width, height, 1.0, 0, pixbuf);

  ret = gdk_pixbuf_compare(pixbuf, page_name);
  
  thumb = poppler_page_get_thumbnail(page);
  if (thumb)
    ret |= gdk_pixbuf_compare(thumb, thumb_name);
 
  if (thumb)
    g_object_unref (G_OBJECT (thumb));
  g_object_unref (G_OBJECT (page));
  g_object_unref (G_OBJECT (pixbuf));

  free(page_format);
  free(page_name);
  return ret;
}

void poppler_test(char *pdf_file)
{
  PopplerDocument *document;
  char *title, *label;
  GError *error;
  GList *list, *l;
  char *uri;
  char *srcdir;
  poppler_test_status_t ret;

  /* build an absolute url for poppler_document_new_from_file */
  gchar * cwd = g_get_current_dir();
  asprintf (&uri, "file://%s/%s", cwd, pdf_file);
  g_free(cwd);

  error = NULL;
  document = poppler_document_new_from_file (uri, NULL, &error);
  if (document == NULL)
    FAIL (error->message);
  int i;
  int n_pages = poppler_document_get_n_pages(document);
  for (i=0; i<n_pages; i++) {
    printf("%s-%d ", pdf_file, i);
    fflush(stdout);
    if (poppler_test_page(pdf_file, document, i, n_pages))
      printf("FAIL\n");
    else
      printf("PASS\n");
  }
  g_object_unref (G_OBJECT (document));
  gfree (uri);
}

int main(int argc, char **argv)
{
  glob_t globbuf;
  int i;
  struct timeval start;
  struct timeval end;
  int total_secs;
  g_type_init ();

  gettimeofday(&start, NULL);
  if (argc > 1) {
	  cache_init(argv[1]);
	  for (i=1; i<argc; i++) {
		  poppler_test(argv[i]);
	  }
  } else {
	  cache_init("tests/");
	  glob("tests/*.pdf", 0, NULL, &globbuf);
	  for (i=0; i<globbuf.gl_pathc; i++) {
		  poppler_test(globbuf.gl_pathv[i]);
	  }
	  globfree(&globbuf);
  }
  gettimeofday(&end, NULL);

  total_secs = (end.tv_sec - start.tv_sec);
  printf("tests took %dmin %dsec\n", total_secs / 60, total_secs % 60);
  return 0;
}


