#ifndef MOUSE_H
#define MOUSE_H

#include <stddef.h>

/* Initialize mouse support (call once at startup) */
void mouse_init(void);

/* Handle a mouse event and update cursor position if clicked.
   Returns 1 if cursor was moved, 0 otherwise.
   line_num_width: width of line number column to account for offset */
int mouse_handle_click(size_t *cx, size_t *cy, size_t *rowoff, size_t *coloff,
                       size_t line_count, int line_num_width, int max_display);

#endif /* MOUSE_H */
