#include "mouse.h"
#include <curses.h>

void mouse_init(void)
{
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
}

int mouse_handle_click(size_t *cx, size_t *cy, size_t *rowoff, size_t *coloff,
                       size_t line_count, int line_num_width, int max_display)
{
    /* PDCurses API: request_mouse_pos() updates Mouse_status */
    request_mouse_pos();

    int click_y = MOUSE_Y_POS;
    int click_x = MOUSE_X_POS;

    /* Only handle left button press/release */
    if (!(Mouse_status.button[0] & (BUTTON_PRESSED | BUTTON_RELEASED)))
        return 0;

    /* Ignore clicks on status/command lines */
    if (click_y >= max_display)
        return 0;

    /* Account for line number column */
    if (click_x < line_num_width)
        click_x = line_num_width;

    /* Calculate buffer position */
    size_t new_cy = *rowoff + (size_t)click_y;
    size_t new_cx = *coloff + (size_t)(click_x - line_num_width);

    /* Bounds check */
    if (new_cy >= line_count)
        new_cy = line_count - 1;

    *cy = new_cy;
    *cx = new_cx;

    return 1;
}
