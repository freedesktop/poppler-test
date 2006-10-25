void cache_init(const char *path);
int cache_compare(const char *path, const unsigned char *buffer, unsigned int length);
FILE *cache_open(const char *path, const char *mode);
void cache_stats_print(void);
