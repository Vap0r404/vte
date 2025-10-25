#include "line_edit.h"
#include <stdlib.h>
#include <string.h>

static size_t max_size(size_t a, size_t b) { return a > b ? a : b; }

void le_init(LineEdit *le, const char *src)
{
    if (!le)
        return;
    size_t sl = src ? strlen(src) : 0;
    size_t cap = max_size((size_t)16, sl + 1);
    le->buf = (char *)malloc(cap);
    if (!le->buf)
    {
        le->len = le->cap = le->pos = 0;
        return;
    }
    if (src)
        memcpy(le->buf, src, sl);
    le->buf[sl] = '\0';
    le->len = sl;
    le->cap = cap;
    le->pos = sl; /* default cursor at end */
}

void le_free(LineEdit *le)
{
    if (!le)
        return;
    free(le->buf);
    le->buf = NULL;
    le->len = le->cap = le->pos = 0;
}

static int ensure_capacity(LineEdit *le, size_t needed)
{
    if (needed <= le->cap)
        return 1;
    size_t newcap = max_size(le->cap * 2, needed);
    char *n = (char *)realloc(le->buf, newcap);
    if (!n)
        return 0;
    le->buf = n;
    le->cap = newcap;
    return 1;
}

int le_insert_char(LineEdit *le, int ch)
{
    if (!le || !le->buf)
        return 0;
    if (le->len + 1 + 1 > le->cap)
    {
        if (!ensure_capacity(le, le->len + 2))
            return 0;
    }
    /* shift right using memmove for better performance */
    if (le->pos < le->len)
        memmove(le->buf + le->pos + 1, le->buf + le->pos, le->len - le->pos);
    le->buf[le->pos] = (char)ch;
    le->len++;
    le->pos++;
    le->buf[le->len] = '\0';
    return 1;
}

int le_backspace(LineEdit *le)
{
    if (!le || !le->buf)
        return 0;
    if (le->pos == 0)
        return 0;
    /* remove char before pos using memmove */
    if (le->pos < le->len)
        memmove(le->buf + le->pos - 1, le->buf + le->pos, le->len - le->pos);
    le->pos--;
    le->len--;
    le->buf[le->len] = '\0';
    return 1;
}

int le_delete(LineEdit *le)
{
    if (!le || !le->buf)
        return 0;
    if (le->pos >= le->len)
        return 0;
    /* remove char at pos */
    if (le->pos < le->len - 1)
        memmove(le->buf + le->pos, le->buf + le->pos + 1, le->len - le->pos - 1);
    le->len--;
    le->buf[le->len] = '\0';
    return 1;
}

void le_move_left(LineEdit *le)
{
    if (le && le->pos > 0)
        le->pos--;
}
void le_move_right(LineEdit *le)
{
    if (le && le->pos < le->len)
        le->pos++;
}
void le_move_home(LineEdit *le)
{
    if (le)
        le->pos = 0;
}
void le_move_end(LineEdit *le)
{
    if (le)
        le->pos = le->len;
}

char *le_split(LineEdit *le)
{
    if (!le || !le->buf)
        return NULL;
    size_t right_len = le->len - le->pos;
    char *right = (char *)malloc(right_len + 1);
    if (!right)
        return NULL;
    if (right_len > 0)
        memcpy(right, le->buf + le->pos, right_len);
    right[right_len] = '\0';
    /* truncate left side */
    le->buf[le->pos] = '\0';
    le->len = le->pos;
    return right;
}

char *le_take_string(const LineEdit *le)
{
    if (!le || !le->buf)
        return NULL;
    char *out = (char *)malloc(le->len + 1);
    if (!out)
        return NULL;
    memcpy(out, le->buf, le->len);
    out[le->len] = '\0';
    return out;
}
