#include "navigation.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void nav_init(NavState *nav)
{
    nav->last_search[0] = '\0';
    nav->line_num_width = 4;
}

int nav_calc_line_num_width(size_t line_count)
{
    int width = snprintf(NULL, 0, "%zu", line_count) + 1;
    return width < 4 ? 4 : width;
}

int nav_goto_line(size_t target_line, size_t *cy, size_t *cx, size_t *rowoff,
                  size_t max_display, size_t line_count)
{
    if (target_line == 0 || target_line > line_count)
        return 0;

    *cy = target_line - 1;
    *cx = 0;

    /* Adjust scroll */
    if (*cy < *rowoff)
        *rowoff = *cy;
    if (*cy >= *rowoff + max_display)
        *rowoff = *cy - max_display + 1;

    return 1;
}

int nav_search_forward(const char *pattern, Buffer *buf, size_t *cy, size_t *cx,
                       size_t *rowoff, size_t max_display, NavState *nav)
{
    if (!pattern || pattern[0] == '\0')
        return 0;

    /* Save search pattern */
    strncpy(nav->last_search, pattern, sizeof(nav->last_search) - 1);
    nav->last_search[sizeof(nav->last_search) - 1] = '\0';

    size_t start_line = *cy;
    size_t start_col = *cx + 1;

    /* Search from current position to end */
    for (size_t search_line = start_line; search_line < buf->count; ++search_line)
    {
        const char *search_start = buf->lines[search_line];
        if (search_line == start_line)
            search_start += start_col;

        const char *pos = strstr(search_start, pattern);
        if (pos)
        {
            *cy = search_line;
            *cx = pos - buf->lines[search_line];

            /* Adjust scroll */
            if (*cy < *rowoff)
                *rowoff = *cy;
            if (*cy >= *rowoff + max_display)
                *rowoff = *cy - max_display + 1;

            return 1;
        }
    }

    /* Wrap around to beginning */
    for (size_t search_line = 0; search_line <= start_line; ++search_line)
    {
        const char *search_start = buf->lines[search_line];
        size_t search_end = strlen(buf->lines[search_line]);
        if (search_line == start_line)
            search_end = *cx;

        const char *pos = strstr(search_start, pattern);
        if (pos && (pos - search_start) < (int)search_end)
        {
            *cy = search_line;
            *cx = pos - buf->lines[search_line];

            /* Adjust scroll */
            if (*cy < *rowoff)
                *rowoff = *cy;
            if (*cy >= *rowoff + max_display)
                *rowoff = *cy - max_display + 1;

            return 2; /* wrapped */
        }
    }

    return 0; /* not found */
}

int nav_search_next(Buffer *buf, size_t *cy, size_t *cx, size_t *rowoff,
                    size_t max_display, NavState *nav)
{
    if (nav->last_search[0] == '\0')
        return 0;

    size_t start_line = *cy;
    size_t start_col = *cx + 1;

    /* Search from current position to end */
    for (size_t search_line = start_line; search_line < buf->count; ++search_line)
    {
        const char *search_start = buf->lines[search_line];
        if (search_line == start_line)
            search_start += start_col;

        const char *pos = strstr(search_start, nav->last_search);
        if (pos)
        {
            *cy = search_line;
            *cx = pos - buf->lines[search_line];

            /* Adjust scroll */
            if (*cy < *rowoff)
                *rowoff = *cy;
            if (*cy >= *rowoff + max_display)
                *rowoff = *cy - max_display + 1;

            return 1;
        }
    }

    /* Wrap around to beginning */
    for (size_t search_line = 0; search_line <= start_line; ++search_line)
    {
        const char *search_start = buf->lines[search_line];
        size_t search_end = strlen(buf->lines[search_line]);
        if (search_line == start_line)
            search_end = *cx;

        const char *pos = strstr(search_start, nav->last_search);
        if (pos && (pos - search_start) < (int)search_end)
        {
            *cy = search_line;
            *cx = pos - buf->lines[search_line];

            /* Adjust scroll */
            if (*cy < *rowoff)
                *rowoff = *cy;
            if (*cy >= *rowoff + max_display)
                *rowoff = *cy - max_display + 1;

            return 2; /* wrapped */
        }
    }

    return 0; /* not found */
}

int nav_search_prev(Buffer *buf, size_t *cy, size_t *cx, size_t *rowoff,
                    size_t max_display, NavState *nav)
{
    if (nav->last_search[0] == '\0')
        return 0;

    /* Search backward from current position */
    for (size_t search_line = *cy; search_line-- > 0;)
    {
        const char *line = buf->lines[search_line];
        size_t line_len = strlen(line);
        size_t search_end = line_len;
        if (search_line == *cy && *cx > 0)
            search_end = *cx;

        /* Find last occurrence in line before search_end */
        const char *pos = NULL;
        const char *current = line;
        while (current - line < (int)search_end)
        {
            const char *match = strstr(current, nav->last_search);
            if (!match || match - line >= (int)search_end)
                break;
            pos = match;
            current = match + 1;
        }

        if (pos)
        {
            *cy = search_line;
            *cx = pos - buf->lines[search_line];

            /* Adjust scroll */
            if (*cy < *rowoff)
                *rowoff = *cy;
            if (*cy >= *rowoff + max_display)
                *rowoff = *cy - max_display + 1;

            return 1;
        }
    }

    /* Wrap around to end */
    for (size_t search_line = buf->count; search_line-- > *cy;)
    {
        const char *line = buf->lines[search_line];

        /* Find last occurrence in line */
        const char *pos = NULL;
        const char *current = line;
        while (1)
        {
            const char *match = strstr(current, nav->last_search);
            if (!match)
                break;
            pos = match;
            current = match + 1;
        }

        if (pos)
        {
            *cy = search_line;
            *cx = pos - buf->lines[search_line];

            /* Adjust scroll */
            if (*cy < *rowoff)
                *rowoff = *cy;
            if (*cy >= *rowoff + max_display)
                *rowoff = *cy - max_display + 1;

            return 2; /* wrapped */
        }
    }

    return 0; /* not found */
}
