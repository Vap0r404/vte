#include "mouse.h"
#include "wrap.h"
#include <curses.h>
#include <string.h>

void mouse_init(void)
{
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
}

int mouse_handle_click(size_t *cx, size_t *cy, size_t *rowoff,
                       char **lines, size_t line_count,
                       int line_num_width, int max_display, int text_width)
{
    /* PDCurses: getmouse() returns button event mask; also refresh Mouse_status coords */
    mmask_t mev = getmouse();
    if (!(mev & (BUTTON1_CLICKED | BUTTON1_DOUBLE_CLICKED)))
        return 0;
    request_mouse_pos();
    int click_y = MOUSE_Y_POS;
    int click_x = MOUSE_X_POS;

    /* Ignore clicks on status/command lines */
    if (click_y >= max_display)
        return 0;

    if (text_width < 1)
        text_width = 1;

    /* Translate click to text area X (column) */
    int text_x = click_x - line_num_width;
    if (text_x < 0)
        text_x = 0;

    /* Absolute visual row from top of file */
    size_t abs_vis_row = *rowoff + (size_t)click_y;

    /* Find target buffer line and wrap segment */
    size_t sum = 0;
    size_t target_line = line_count ? (line_count - 1) : 0;
    size_t segment = 0;
    for (size_t i = 0; i < line_count; ++i)
    {
        int vis = wrap_calc_visual_lines(lines[i], text_width);
        if (abs_vis_row < sum + (size_t)vis)
        {
            target_line = i;
            segment = abs_vis_row - sum;
            break;
        }
        sum += (size_t)vis;
    }

    const char *tline = (line_count > 0) ? lines[target_line] : "";
    size_t base_col = segment * (size_t)text_width;
    size_t target_col = base_col + (size_t)text_x;
    size_t new_cx = wrap_byte_index_for_col(tline, (int)target_col);
    size_t new_cy = target_line;

    /* Bounds check */
    if (line_count == 0)
    {
        new_cy = 0;
        new_cx = 0;
    }
    else if (new_cy >= line_count)
    {
        new_cy = line_count - 1;
    }

    *cy = new_cy;
    *cx = new_cx;

    return 1;
}
