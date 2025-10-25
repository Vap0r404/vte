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

int wrap_draw_line(const char *line, int row, int col_start, int width, size_t coloff)
{
    if (!line || width <= 0)
        return 0;

    int len = (int)strlen(line);
    if ((int)coloff >= len)
        return 0;

    const char *text = line + coloff;
    int remaining = len - (int)coloff;
    int rows_used = 0;

    while (remaining > 0 && rows_used < 100) /* safety limit */
    {
        int to_print = remaining;
        if (to_print > width)
            to_print = width;

        mvprintw(row + rows_used, col_start, "%.*s", to_print, text);

        text += to_print;
        remaining -= to_print;
        rows_used++;

        if (remaining <= 0)
            break;
    }

    return rows_used > 0 ? rows_used : 1;
}
