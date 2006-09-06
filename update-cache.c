#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include <string.h>
#include <openssl/sha.h>
#include <libgen.h>
#include "read-png.h"
#include "read-cache.h"

static void hash_image(FILE *cache, const char *path)
{
  unsigned int width, height, stride;
  unsigned char *buf, *buf_diff;
  unsigned char hash[SHA_DIGEST_LENGTH];
  unsigned int buffer_length;
  read_png_status_t status;
  char *path_copy = strdup(path);
  char *name = basename(path_copy);

  status = read_png_argb32 (path, &buf, &width, &height, &stride);
  buffer_length = height * stride;
  printf("%d %d %d\n", width, height, stride);
  SHA1(buf, buffer_length, hash);
  int i;
  for (i=0; i<sizeof(hash); i++) {
    printf("%02x", hash[i]);
  }
  printf("\n");
  /* write the NULL character as well */
  fwrite(name, 1, strlen(name)+1, cache);
  fwrite(hash, 1, sizeof(hash), cache);

  free(path_copy);
  free(buf);
}

int main(int argc, char **argv)
{
  glob_t globbuf;
  int i;
  FILE *cache;

  if (argc > 1) {
    /* search in directory of first argument for cache file */
    cache = cache_open(argv[1], "w+");
  } else {
    cache = cache_open("test/", "w+");
  }
  if (!cache) {
    printf("could not open cache\n");
    exit(1);
  }
  if (argc > 1) {
    for (i=1; i<argc; i++) {
      char *glob_pat;
      asprintf(&glob_pat, "%s*-ref.png", argv[i]);
      printf("pat: %s\n", glob_pat);
      glob(glob_pat, 0, NULL, &globbuf);
      int j;
      for (j=0; j<globbuf.gl_pathc; j++) {
	printf("%s\n", globbuf.gl_pathv[j]);
	hash_image(cache, globbuf.gl_pathv[j]);
      }
      globfree(&globbuf);
      free(glob_pat);
    }
  } else {
    glob("tests/*.pdf*-ref.png", 0, NULL, &globbuf);
    for (i=0; i<globbuf.gl_pathc; i++) {
      printf("%s\n", globbuf.gl_pathv[i]);
      hash_image(cache, globbuf.gl_pathv[i]);
    }
    globfree(&globbuf);
  }
  fclose(cache);

  return 0;
}
