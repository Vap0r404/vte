#include "wrap.h"
#include <curses.h>
#include <string.h>

int wrap_calc_visual_lines(const char *line, int width)
{
    if (width <= 0)
        return 1;

    int len = (int)strlen(line);
    if (len == 0)
        return 1;

    return (len + width - 1) / width;
}

int wrap_draw_line(const char *line, int row, int col_start, int width, size_t coloff, int max_rows)
{
    if (!line || width <= 0)
        return 0;

    int len = (int)strlen(line);
    if ((int)coloff >= len)
        return 0;

    const char *text = line + coloff;
    int remaining = len - (int)coloff;
    int rows_used = 0;

    if (max_rows <= 0)
        max_rows = 1;

    while (remaining > 0 && rows_used < max_rows)
    {
        int to_print = remaining;
        if (to_print > width)
            to_print = width;

        /* Print exactly 'width' cells, padding with spaces, so we never overdraw the gutter
           and we overwrite any leftovers without using clrtoeol(). */
        mvprintw(row + rows_used, col_start, "%-*.*s", width, to_print, text);

        text += to_print;
        remaining -= to_print;
        rows_used++;
    }

    return rows_used > 0 ? rows_used : 1;
}
