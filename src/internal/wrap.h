#ifndef WRAP_H
#define WRAP_H

#include <stddef.h>

/* Calculate how many visual lines a text line will occupy when wrapped.
   width: available display width for text (excluding line numbers) */
int wrap_calc_visual_lines(const char *line, int width);

/* Draw a wrapped line starting at screen row 'row'.
   Returns the number of screen rows used.
   Handles lines that exceed terminal width by wrapping to next line. */
/* Draw a wrapped line starting at screen row 'row'.
   max_rows limits how many screen rows may be written (clipped to viewport).
   Returns the number of screen rows used (clipped to max_rows, at least 1 if any text shown). */
int wrap_draw_line(const char *line, int row, int col_start, int width, size_t coloff, int max_rows);

#endif /* WRAP_H */
