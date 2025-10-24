#ifndef LINE_EDIT_H
#define LINE_EDIT_H

#include <stddef.h>

typedef struct {
    char *buf;    /* NUL-terminated buffer owned by struct */
    size_t len;   /* length (bytes) excluding NUL */
    size_t cap;   /* allocated capacity including NUL */
    size_t pos;   /* cursor position in [0..len] */
} LineEdit;

void le_init(LineEdit *le, const char *src);
/* Free internal memory (safe to call on uninitialized fields after init). */
void le_free(LineEdit *le);
/* Insert a character at the current cursor position. Returns 1 on success, 0 on failure. */
int le_insert_char(LineEdit *le, int ch);
int le_backspace(LineEdit *le);
void le_move_left(LineEdit *le);
void le_move_right(LineEdit *le);
/* Split the buffer at current cursor position. The right side is returned as a malloc'ed string (caller frees). */
char *le_split(LineEdit *le);
/* Return a malloc'ed copy of the current buffer and leave le unchanged. Caller frees. */
char *le_take_string(const LineEdit *le);

#endif /* LINE_EDIT_H */
