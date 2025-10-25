#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stddef.h>
#include "buffer.h"

/* Navigation state */
typedef struct
{
    char last_search[256];
    int line_num_width;
} NavState;

/* Initialize navigation state */
void nav_init(NavState *nav);

/* Calculate line number width based on buffer line count */
int nav_calc_line_num_width(size_t line_count);

/* Goto line command - returns 1 if successful, 0 if invalid */
int nav_goto_line(size_t target_line, size_t *cy, size_t *cx, size_t *rowoff,
                  size_t max_display, size_t line_count);

/* Search forward for pattern - returns 1 if found, 0 if not found, 2 if wrapped */
int nav_search_forward(const char *pattern, Buffer *buf, size_t *cy, size_t *cx,
                       size_t *rowoff, size_t max_display, NavState *nav);

/* Repeat last search forward - returns 1 if found, 0 if not found, 2 if wrapped */
int nav_search_next(Buffer *buf, size_t *cy, size_t *cx, size_t *rowoff,
                    size_t max_display, NavState *nav);

/* Repeat last search backward - returns 1 if found, 0 if not found, 2 if wrapped */
int nav_search_prev(Buffer *buf, size_t *cy, size_t *cx, size_t *rowoff,
                    size_t max_display, NavState *nav);

#endif /* NAVIGATION_H */
