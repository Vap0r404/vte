#ifndef UTF8_H
#define UTF8_H

#include <stddef.h>

/* Read a UTF-8 character from getch() and return the unicode codepoint.
   Returns the codepoint, or -1 on error.
   For ASCII (< 128), returns the character directly.
   For multi-byte UTF-8, reads additional bytes as needed. */
int utf8_getch(void);

#endif /* UTF8_H */
