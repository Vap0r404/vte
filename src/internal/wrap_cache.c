#include <stdlib.h>
#include "wrap_cache.h"
#include "wrap.h"

void wrap_cache_init(WrapCache *c, size_t count)
{
    c->width = -1;
    c->cap = count ? count : 1;
    c->count = count;
    c->counts = (int *)malloc(c->cap * sizeof(int));
    for (size_t i = 0; i < c->cap; ++i)
        c->counts[i] = -1;
}

void wrap_cache_free(WrapCache *c)
{
    if (c->counts)
        free(c->counts);
    c->counts = NULL;
    c->cap = c->count = 0;
    c->width = -1;
}

void wrap_cache_set_width(WrapCache *c, int width)
{
    if (c->width != width)
    {
        c->width = width;
        for (size_t i = 0; i < c->count; ++i)
            c->counts[i] = -1;
    }
}

void wrap_cache_ensure(WrapCache *c, size_t count)
{
    if (count > c->cap)
    {
        size_t new_cap = c->cap ? c->cap : 1;
        while (new_cap < count)
            new_cap *= 2;
        c->counts = (int *)realloc(c->counts, new_cap * sizeof(int));
        for (size_t i = c->cap; i < new_cap; ++i)
            c->counts[i] = -1;
        c->cap = new_cap;
    }
    c->count = count;
}

void wrap_cache_invalidate_line(WrapCache *c, size_t idx)
{
    if (idx < c->count)
        c->counts[idx] = -1;
}

void wrap_cache_invalidate_all(WrapCache *c)
{
    for (size_t i = 0; i < c->count; ++i)
        c->counts[i] = -1;
}

int wrap_cache_get(WrapCache *c, const char *line, size_t idx)
{
    int width = c->width < 1 ? 1 : c->width;
    if (idx >= c->count)
        return wrap_calc_visual_lines(line, width);
    int v = c->counts[idx];
    if (v >= 0)
        return v;
    v = wrap_calc_visual_lines(line, width);
    c->counts[idx] = v;
    return v;
}
