#include "undo.h"
#include <stdlib.h>
#include <string.h>

#define MAX_UNDO_STACK 1000

static UndoAction undo_stack[MAX_UNDO_STACK];
static size_t undo_count = 0;

static UndoAction redo_stack[MAX_UNDO_STACK];
static size_t redo_count = 0;

void undo_init(void)
{
    undo_count = 0;
    redo_count = 0;
}

static void free_action(UndoAction *action)
{
    if (action->data)
    {
        free(action->data);
        action->data = NULL;
    }
    if (action->data2)
    {
        free(action->data2);
        action->data2 = NULL;
    }
}

void undo_free(void)
{
    for (size_t i = 0; i < undo_count; ++i)
        free_action(&undo_stack[i]);
    for (size_t i = 0; i < redo_count; ++i)
        free_action(&redo_stack[i]);
    undo_count = 0;
    redo_count = 0;
}

static void push_action(UndoActionType type, size_t line, size_t pos, const char *data, const char *data2)
{
    if (undo_count >= MAX_UNDO_STACK)
    {
        /* Stack full, discard oldest */
        free_action(&undo_stack[0]);
        memmove(&undo_stack[0], &undo_stack[1], sizeof(UndoAction) * (MAX_UNDO_STACK - 1));
        undo_count = MAX_UNDO_STACK - 1;
    }

    UndoAction *action = &undo_stack[undo_count++];
    action->type = type;
    action->line = line;
    action->pos = pos;
    action->data = data ? strdup(data) : NULL;
    action->data2 = data2 ? strdup(data2) : NULL;
}

void undo_push_insert_char(size_t line, size_t pos, const char *ch_utf8)
{
    push_action(UNDO_INSERT_CHAR, line, pos, ch_utf8, NULL);
}

void undo_push_delete_char(size_t line, size_t pos, const char *ch_utf8)
{
    push_action(UNDO_DELETE_CHAR, line, pos, ch_utf8, NULL);
}

void undo_push_insert_line(size_t line, const char *left, const char *right)
{
    push_action(UNDO_INSERT_LINE, line, 0, left, right);
}

void undo_push_delete_line(size_t line, const char *deleted)
{
    push_action(UNDO_DELETE_LINE, line, 0, deleted, NULL);
}

void undo_push_replace_line(size_t line, const char *old_content)
{
    push_action(UNDO_REPLACE_LINE, line, 0, old_content, NULL);
}

void undo_push_replace_line_full(size_t line, const char *old_content, const char *new_content)
{
    push_action(UNDO_REPLACE_LINE, line, 0, old_content, new_content);
}

int undo_can_undo(void)
{
    return undo_count > 0;
}

int undo_can_redo(void)
{
    return redo_count > 0;
}

const UndoAction *undo_peek(void)
{
    if (undo_count == 0)
        return NULL;
    return &undo_stack[undo_count - 1];
}

const UndoAction *redo_peek(void)
{
    if (redo_count == 0)
        return NULL;
    return &redo_stack[redo_count - 1];
}

void undo_pop(void)
{
    if (undo_count > 0)
    {
        undo_count--;
        free_action(&undo_stack[undo_count]);
    }
}

void redo_pop(void)
{
    if (redo_count > 0)
    {
        redo_count--;
        free_action(&redo_stack[redo_count]);
    }
}

void undo_push_to_redo(UndoAction *action)
{
    if (redo_count >= MAX_UNDO_STACK)
    {
        /* Stack full, discard oldest */
        free_action(&redo_stack[0]);
        memmove(&redo_stack[0], &redo_stack[1], sizeof(UndoAction) * (MAX_UNDO_STACK - 1));
        redo_count = MAX_UNDO_STACK - 1;
    }

    redo_stack[redo_count++] = *action;
}

void redo_push_to_undo(UndoAction *action)
{
    if (undo_count >= MAX_UNDO_STACK)
    {
        /* Stack full, discard oldest */
        free_action(&undo_stack[0]);
        memmove(&undo_stack[0], &undo_stack[1], sizeof(UndoAction) * (MAX_UNDO_STACK - 1));
        undo_count = MAX_UNDO_STACK - 1;
    }

    undo_stack[undo_count++] = *action;
}

void undo_clear_redo(void)
{
    for (size_t i = 0; i < redo_count; ++i)
        free_action(&redo_stack[i]);
    redo_count = 0;
}
