#ifndef VTE_WRAP_CACHE_H
#define VTE_WRAP_CACHE_H

#include <stddef.h>

/* Cache of per-line visual wrap counts at a specific text width */
typedef struct WrapCache
{
    int width;    /* cached text width; -1 when invalid */
    size_t cap;   /* allocated size of counts array */
    size_t count; /* logical number of lines tracked */
    int *counts;  /* per-line visual line counts; -1 means unknown */
} WrapCache;

void wrap_cache_init(WrapCache *c, size_t count);
void wrap_cache_free(WrapCache *c);
void wrap_cache_set_width(WrapCache *c, int width);
void wrap_cache_ensure(WrapCache *c, size_t count);
void wrap_cache_invalidate_line(WrapCache *c, size_t idx);
void wrap_cache_invalidate_all(WrapCache *c);
int wrap_cache_get(WrapCache *c, const char *line, size_t idx);

#endif /* VTE_WRAP_CACHE_H */
