#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <poppler.h>
#include <glob.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "read-cache.h"
#include "buffer-diff.h"
#define FAIL(msg) \
	do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)


#define POPPLER_TEST_LOG_SUFFIX ".log"
#define POPPLER_TEST_PNG_SUFFIX "-out.png"
#define POPPLER_TEST_REF_PNG_SUFFIX "-ref.png"
#define POPPLER_TEST_DIFF_PNG_SUFFIX "-diff.png"
#define POPPLER_TEST_TXT_SUFFIX "-out.txt"
#define POPPLER_TEST_REF_TXT_SUFFIX "-ref.txt"
#define POPPLER_TEST_DIFF_TXT_SUFFIX "-diff.txt"
#define POPPLER_TEST_PS_SUFFIX "-out.ps"
#define POPPLER_TEST_REF_PS_SUFFIX "-ref.ps"
#define POPPLER_TEST_DIFF_PS_SUFFIX "-diff.ps"


bool text_output = false;
bool ps_output = false;

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

struct diff_results gdk_pixbuf_compare(GdkPixbuf *pixbuf, char *page_name)
{
  char *png_name, *ref_name, *diff_name;
  char *srcdir;
  struct diff_results results;
  const char *backend = poppler_get_backend() == POPPLER_BACKEND_SPLASH ? "splash" : "cairo";
   /* Get the strings ready that we'll need. */
  srcdir = getenv ("srcdir");
  if (!srcdir)
    srcdir = ".";
  asprintf (&png_name, "%s-%s%s", page_name, backend, POPPLER_TEST_PNG_SUFFIX);
  asprintf (&ref_name, "%s/%s-%s%s", srcdir, page_name, backend, POPPLER_TEST_REF_PNG_SUFFIX);
  asprintf (&diff_name, "%s-%s%s", page_name, backend, POPPLER_TEST_DIFF_PNG_SUFFIX);

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

  results = image_buf_diff (cairo_pixels, width, height, width*4, png_name, ref_name, diff_name);

  free (cairo_pixels);
  free (png_name);
  free (ref_name);
  free (diff_name);

  return results;
}

PopplerPSFile *poppler_ps_file_new            (PopplerDocument *document,
                                               const char      *filename,
                                               int              first_page,
                                               int              n_pages);
void           poppler_ps_file_set_paper_size (PopplerPSFile   *ps_file,
                                               double           width,
                                               double           height);
void           poppler_ps_file_set_duplex     (PopplerPSFile   *ps_file,
                                               gboolean         duplex);
void           poppler_ps_file_free           (PopplerPSFile   *ps_file);

poppler_test_status_t poppler_test_document_ps(PopplerDocument *document, char *postscript_name)
{
  char *ps_name, *ref_name, *diff_name;
  char *srcdir;
  int i;
  poppler_test_status_t ret = POPPLER_TEST_SUCCESS;

  /* Get the strings ready that we'll need. */
  srcdir = getenv ("srcdir");
  if (!srcdir)
    srcdir = ".";

  asprintf (&ps_name, "%s%s", postscript_name, POPPLER_TEST_PS_SUFFIX);
  asprintf (&ref_name, "%s/%s%s", srcdir, postscript_name, POPPLER_TEST_REF_PS_SUFFIX);
  asprintf (&diff_name, "%s%s", postscript_name, POPPLER_TEST_DIFF_PS_SUFFIX);
  int n_pages = poppler_document_get_n_pages(document);
  PopplerPSFile *ps_file = poppler_ps_file_new(document, ps_name, 0, poppler_document_get_n_pages(document));
  for (i=0; i<n_pages; i++) {
    PopplerPage *page = poppler_document_get_page (document, i);
    poppler_page_render_to_ps (page, ps_file);
  }
  /* we need to free before comparing otherwise the document might not be flushed */
  poppler_ps_file_free(ps_file);

  {
    char *diff_cmd;
    asprintf (&diff_cmd, "diff -u %s %s > %s", ps_name, ref_name, diff_name);
    int diff_ret = WEXITSTATUS(system(diff_cmd));
    if (diff_ret == 0 || diff_ret == 2 /* file not found */) {
      xunlink (diff_name);
    }
    if (diff_ret)
      ret = POPPLER_TEST_FAILURE;
    free(diff_cmd);
  }

  free(ps_name);
  free(ref_name);
  free(diff_name);
  return ret;
}

poppler_test_status_t poppler_test_page_text(PopplerPage *page, char *text_name)
{
  double width, height;
  char *txt_name, *ref_name, *diff_name;
  char *srcdir;
  poppler_test_status_t ret = POPPLER_TEST_SUCCESS;
  PopplerRectangle *rect = poppler_rectangle_new();
  poppler_page_get_size (page, &width, &height);

  /* Get the strings ready that we'll need. */
  srcdir = getenv ("srcdir");
  if (!srcdir)
    srcdir = ".";

  asprintf (&txt_name, "%s%s", text_name, POPPLER_TEST_TXT_SUFFIX);
  asprintf (&ref_name, "%s/%s%s", srcdir, text_name, POPPLER_TEST_REF_TXT_SUFFIX);
  asprintf (&diff_name, "%s%s", text_name, POPPLER_TEST_DIFF_TXT_SUFFIX);
  rect->x1 = rect->y1 = 0.0;
  rect->x2 = width;
  rect->y2 = height;
  char *text = poppler_page_get_text(page, rect);
  int len = strlen(text);
  if (len) {
    FILE *file = fopen(txt_name, "wb");
    char *diff_cmd;
    fwrite(text, 1, len, file);
    fclose(file);
    asprintf (&diff_cmd, "diff -u %s %s > %s", txt_name, ref_name, diff_name);
    int diff_ret = WEXITSTATUS(system(diff_cmd));
    if (diff_ret == 0 || diff_ret == 2 /* file not found */) {
      xunlink (diff_name);
    }
    if (diff_ret)
      ret = POPPLER_TEST_FAILURE;
    free(diff_cmd);
  }
  g_free(text);
  poppler_rectangle_free(rect);

  free(txt_name);
  free(ref_name);
  free(diff_name);
  return ret;
}

poppler_test_status_t poppler_test_page(char *pdf_file, PopplerDocument *document, int page_index, int n_pages, struct diff_results *results) {
  GdkPixbuf *pixbuf, *thumb;
  double width, height;
  PopplerPage *page;
  poppler_test_status_t ret;
  char *page_format;
  char *page_name;
  char *thumb_name;
  char *text_name;

  asprintf(&page_format, "%%s-%%0%dd", ilog10(n_pages)+1);
  asprintf(&page_name, page_format, pdf_file, page_index);
  asprintf(&thumb_name, "%s-thumb", page_name);
  asprintf(&text_name, "%s-text", page_name);

  page = poppler_document_get_page (document, page_index);
  if (!page)
    FAIL ("page not found");

  poppler_page_get_size (page, &width, &height);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
  poppler_page_render_to_pixbuf (page, 0, 0, width, height, 1.0, 0, pixbuf);

  *results = gdk_pixbuf_compare(pixbuf, page_name);
  ret = results->pixels_changed ? POPPLER_TEST_FAILURE : POPPLER_TEST_SUCCESS;

  if (text_output)
    ret |= poppler_test_page_text(page, text_name);

  thumb = poppler_page_get_thumbnail(page);
  if (thumb)
    ret |= gdk_pixbuf_compare(thumb, thumb_name).pixels_changed ? POPPLER_TEST_FAILURE : POPPLER_TEST_SUCCESS;
 
  if (thumb)
    g_object_unref (G_OBJECT (thumb));
  g_object_unref (G_OBJECT (page));
  g_object_unref (G_OBJECT (pixbuf));

  free(page_format);
  free(page_name);
  free(thumb_name);
  free(text_name);
  return ret;
}

void poppler_test(char *pdf_file)
{
  PopplerDocument *document;
  GError *error;
  char *uri;

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
    struct diff_results results;
    printf("%s-%d ", pdf_file, i);
    fflush(stdout);
    if (poppler_test_page(pdf_file, document, i, n_pages, &results))
      printf("FAIL (%d %d)\n", results.pixels_changed, results.max_difference);
    else
      printf("PASS\n");
  }
  if (ps_output) {
    printf("%s-ps ", pdf_file);
    fflush(stdout);
    if (poppler_test_document_ps(document, pdf_file))
      printf("FAIL\n");
    else
      printf("PASS\n");
  }

  g_object_unref (G_OBJECT (document));
  g_free (uri);
}

int main(int argc, char **argv)
{
  glob_t globbuf;
  int i;
  struct timeval start;
  struct timeval end;
  int total_secs;
  g_type_init ();
  int opt;
  while ((opt = getopt(argc, argv, "tp")) != -1) {
    switch (opt) {
      case 't':
	text_output = true;
	break;
      case 'p':
	ps_output = true;
	break;
    }
  }
  gettimeofday(&start, NULL);
  if ((argc - optind) >= 1) {
	  cache_init(argv[optind]);
	  for (i=optind; i<argc; i++) {
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
  cache_stats_print();
  return 0;
}


