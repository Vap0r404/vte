#ifndef VTE_CLIPBOARD_H
#define VTE_CLIPBOARD_H

#include <stddef.h>

/* Clipboard type */
typedef enum
{
    CLIP_CHAR, /* Character/word clipboard */
    CLIP_LINE, /* Line clipboard */
} ClipboardType;

/* Initialize clipboard system */
void clipboard_init(void);
void clipboard_free(void);

/* Yank (copy) operations */
void clipboard_yank_char(const char *text);
void clipboard_yank_line(const char *text);

/* Paste operations - returns malloc'ed string (caller frees), or NULL if empty */
char *clipboard_paste(void);
ClipboardType clipboard_type(void);
int clipboard_has_content(void);

#endif /* VTE_CLIPBOARD_H */
