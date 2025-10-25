#ifndef MOUSE_H
#define MOUSE_H

#include <stddef.h>

/* Initialize mouse support (call once at startup) */
void mouse_init(void);

/* Handle a mouse event and update cursor position if clicked.
   Returns 1 if cursor was moved, 0 otherwise.
   line_num_width: width of line number column to account for offset */
/* Wrapped rendering aware: rowoff is visual-row offset, coloff can be 0 when wrapping is enabled.
   lines: array of pointers to buffer lines; line_count: number of lines in buffer.
   text_width: width of the text area (cols - line_num_width). */
int mouse_handle_click(size_t *cx, size_t *cy, size_t *rowoff,
                       char **lines, size_t line_count,
                       int line_num_width, int max_display, int text_width);

#endif /* MOUSE_H */
