#ifndef VTE_UNDO_H
#define VTE_UNDO_H

#include <stddef.h>

/* Undo action types */
typedef enum
{
    UNDO_INSERT_CHAR,  /* Single character insertion */
    UNDO_DELETE_CHAR,  /* Single character deletion */
    UNDO_INSERT_LINE,  /* Line break/split */
    UNDO_DELETE_LINE,  /* Line join/deletion */
    UNDO_REPLACE_LINE, /* Full line replacement */
} UndoActionType;

/* A single undo action */
typedef struct
{
    UndoActionType type;
    size_t line; /* Line number where action occurred */
    size_t pos;  /* Position within line (for char ops) */
    char *data;  /* Saved data (character, line content, etc.) */
    char *data2; /* Optional second data field (for line splits) */
} UndoAction;

/* Undo stack management */
void undo_init(void);
void undo_free(void);

/* Record actions */
void undo_push_insert_char(size_t line, size_t pos, const char *ch_utf8);
void undo_push_delete_char(size_t line, size_t pos, const char *ch_utf8);
void undo_push_insert_line(size_t line, const char *left, const char *right);
void undo_push_delete_line(size_t line, const char *deleted);
void undo_push_replace_line(size_t line, const char *old_content);
void undo_push_replace_line_full(size_t line, const char *old_content, const char *new_content);

/* Undo/redo operations - return 1 if action performed, 0 if stack empty */
int undo_can_undo(void);
int undo_can_redo(void);
const UndoAction *undo_peek(void);
const UndoAction *redo_peek(void);
void undo_pop(void);                        /* Remove from undo stack after applying */
void redo_pop(void);                        /* Remove from redo stack after applying */
void undo_push_to_redo(UndoAction *action); /* Move action to redo stack */
void redo_push_to_undo(UndoAction *action); /* Move action from redo to undo stack */

/* Clear redo stack when new action is performed */
void undo_clear_redo(void);

#endif /* VTE_UNDO_H */
