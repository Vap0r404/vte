#ifndef VTE_UTF8_EDIT_H
#define VTE_UTF8_EDIT_H

#include <stddef.h>
#include "../modules/line_edit.h"

/* Move cursor left by one UTF-8 codepoint (if possible) */
void le_move_left_cp(LineEdit *le);
/* Move cursor right by one UTF-8 codepoint (if possible) */
void le_move_right_cp(LineEdit *le);
/* Backspace one UTF-8 codepoint; returns 1 if something was deleted */
int le_backspace_cp(LineEdit *le);
/* Delete (forward) one UTF-8 codepoint; returns 1 if something was deleted */
int le_delete_cp(LineEdit *le);
/* Insert a Unicode codepoint encoded as UTF-8 at cursor; returns 1 on success */
int le_insert_codepoint(LineEdit *le, int cp);

/* Composition of diacritics to precomposed letters is not handled here.
   Use le_insert_codepoint for direct insertion; platform input method
   should provide composed codepoints when available. */

#endif /* VTE_UTF8_EDIT_H */
