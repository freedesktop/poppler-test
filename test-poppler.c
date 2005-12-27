#include <stdio.h>
#include <stdlib.h>
#include <poppler.h>
#include <glob.h>

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

poppler_test_status_t poppler_test_page(char *pdf_file, PopplerDocument *document, int page_index) {
  char *log_name, *png_name, *ref_name, *diff_name;
  char *srcdir;
  GdkPixbuf *pixbuf, *thumb;
  double width, height;
  PopplerPage *page;
  GError *error;
  const char *backend = poppler_get_backend() == POPPLER_BACKEND_SPLASH ? "splash" : "cairo";
  poppler_test_status_t ret;
  int pixels_changed;
  /* Get the strings ready that we'll need. */
  srcdir = getenv ("srcdir");
  if (!srcdir)
    srcdir = ".";

  error = NULL;
  asprintf (&log_name, "%s-%d-%s%s", pdf_file, page_index, backend, POPPLER_TEST_LOG_SUFFIX);
  asprintf (&png_name, "%s-%d-%s%s", pdf_file, page_index, backend, POPPLER_TEST_PNG_SUFFIX);
  asprintf (&ref_name, "%s/%s-%d-%s%s", srcdir, pdf_file, page_index, backend, POPPLER_TEST_REF_SUFFIX);
  asprintf (&diff_name, "%s-%d-%s%s", pdf_file, page_index, backend, POPPLER_TEST_DIFF_SUFFIX);

  page = poppler_document_get_page (document, page_index);
  if (page == NULL)
    FAIL ("page not found");

  poppler_page_get_size (page, &width, &height);

  pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
  gdk_pixbuf_fill (pixbuf, 0x00106000);
  poppler_page_render_to_pixbuf (page, 0, 0, width, height, 1, 0, pixbuf);

  gdk_pixbuf_save (pixbuf, png_name, "png", &error, NULL);

  if (error != NULL)
    FAIL (error->message);
  g_object_unref (G_OBJECT (page));
  g_object_unref (G_OBJECT (pixbuf));


  pixels_changed = image_diff (png_name, ref_name, diff_name);

  ret = pixels_changed ? POPPLER_TEST_FAILURE : POPPLER_TEST_SUCCESS;
  free (png_name);
  free (ref_name);
  free (diff_name);
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
    if (poppler_test_page(pdf_file, document, i))
      printf("FAIL\n");
    else
      printf("PASS\n");
  }
  gfree (uri);
}

int main(int argc, char **argv)
{
  glob_t globbuf;
  int i;
  g_type_init ();
  if (argc > 1) {
	  for (i=1; i<argc; i++) {
		  poppler_test(argv[i]);
	  }
  } else {
	  glob("tests/*.pdf", 0, NULL, &globbuf);
	  for (i=0; i<globbuf.gl_pathc; i++) {
		  poppler_test(globbuf.gl_pathv[i]);
	  }
  }

  return 0;
}


