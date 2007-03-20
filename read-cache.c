#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <openssl/sha.h>
#include "read-cache.h"

char *cache;
char *cache_end;
char *current_cache_entry;
void cache_init(const char *path)
{
  FILE *f = cache_open(path, "r");
  if (!f) {
    printf("no cache\n");
    return;
  }
  struct stat buf;
  fstat(fileno(f), &buf);
  cache = malloc(buf.st_size);
  fread(cache, 1, buf.st_size, f);
  fclose(f);

  /* set the current entry to the begining of the cache */
  current_cache_entry = cache;
  cache_end = cache + buf.st_size;
}

FILE *cache_open(const char *path, const char *mode)
{
  FILE *cache;
  char *cache_path;
  char *path_copy = strdup(path);
  char *dir = dirname(path_copy);
  asprintf(&cache_path, "%s/cache", dir);
  cache = fopen(cache_path, mode);
  free(cache_path);
  free(path_copy);
  return cache;
}

#define MIN(a, b) (a) < (b) ? (a) : (b)

static void print_hash(const unsigned char *hash)
{
  int i;
  for (i=0; i<SHA_DIGEST_LENGTH; i++) {
    printf("%02x", hash[i]);
  }
  printf("\n");
}

static int cache_misses;
static int cache_hits;

void cache_stats_print(void)
{
  printf("cache: %d of %d hits\n", cache_hits, cache_misses + cache_hits);
}

int cache_compare(const char *path, const unsigned char *buffer, unsigned int length)
{
  char *path_copy = strdup(path);
  char *name = basename(path_copy);
  int match = 0;
  //printf("compare: %s\n", path);
  int name_length = strlen(name);
  while (current_cache_entry < cache_end) {
    int cache_length = strlen(current_cache_entry);
    int min_length = MIN(cache_length, name_length);
    int result = memcmp(name, current_cache_entry, min_length);
    if (result == 0) {
      char *cache_hash = current_cache_entry + cache_length + 1;
      unsigned char hash[SHA_DIGEST_LENGTH];
      //printf("found entry\n");
      SHA1(buffer, length, hash);
      //print_hash(cache_hash);
      //print_hash(hash);
      current_cache_entry = cache_hash + SHA_DIGEST_LENGTH;
      match = memcmp(hash, cache_hash, SHA_DIGEST_LENGTH) == 0 ? 1 : 0;
      //printf("match %d\n", match);
      break;
    } else if (result < 0) {
      break;
    } else {
      //printf("trying next entry: %s vs. %s did not match\n", name, current_cache_entry);
      current_cache_entry += cache_length + 1 + SHA_DIGEST_LENGTH;
    }
  }
  free(path_copy);

  if (match)
    cache_hits++;
  else
    cache_misses++;

  return match;
}
