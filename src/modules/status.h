#ifndef STATUS_H
#define STATUS_H

#include <stddef.h>

/* Format status line with mode, filename, buffer info, and cursor position.
   Returns the formatted string in 'out' buffer. */
void status_format(char *out, size_t len, const char *mode, const char *filename,
                   int buf_index, size_t buf_total, int is_dirty,
                   size_t line, size_t col, int term_cols);

#endif /* STATUS_H */
