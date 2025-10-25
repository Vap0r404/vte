#include "utf8_edit.h"
#include <string.h>

/* Return index of previous UTF-8 codepoint start before 'pos' (0 if none) */
static size_t utf8_prev_cp_start(const char *s, size_t pos)
{
    if (!s || pos == 0)
        return 0;
    size_t i = pos;
    if (i > 0)
        i--;
    /* Move left over continuation bytes 10xxxxxx */
    while (i > 0 && (unsigned char)s[i] >= 0x80 && (unsigned char)s[i] < 0xC0)
        i--;
    return i;
}

/* Return index just after next UTF-8 codepoint starting at or after 'pos' */
static size_t utf8_next_cp_end(const char *s, size_t pos)
{
    if (!s)
        return pos;
    unsigned char b0 = (unsigned char)s[pos];
    if (b0 == 0)
        return pos;
    if (b0 < 0x80)
        return pos + 1;
    if ((b0 & 0xE0) == 0xC0)
    {
        if ((unsigned char)s[pos + 1] >= 0x80 && (unsigned char)s[pos + 1] < 0xC0)
            return pos + 2;
        return pos + 1;
    }
    if ((b0 & 0xF0) == 0xE0)
    {
        if ((unsigned char)s[pos + 1] >= 0x80 && (unsigned char)s[pos + 1] < 0xC0 &&
            (unsigned char)s[pos + 2] >= 0x80 && (unsigned char)s[pos + 2] < 0xC0)
            return pos + 3;
        /* fall back to advance minimally */
        return pos + 1;
    }
    if ((b0 & 0xF8) == 0xF0)
    {
        if ((unsigned char)s[pos + 1] >= 0x80 && (unsigned char)s[pos + 1] < 0xC0 &&
            (unsigned char)s[pos + 2] >= 0x80 && (unsigned char)s[pos + 2] < 0xC0 &&
            (unsigned char)s[pos + 3] >= 0x80 && (unsigned char)s[pos + 3] < 0xC0)
            return pos + 4;
        return pos + 1;
    }
    return pos + 1;
}

void le_move_left_cp(LineEdit *le)
{
    if (!le || le->pos == 0)
        return;
    le->pos = utf8_prev_cp_start(le->buf, le->pos);
}

void le_move_right_cp(LineEdit *le)
{
    if (!le || le->pos >= le->len)
        return;
    le->pos = utf8_next_cp_end(le->buf, le->pos);
    if (le->pos > le->len)
        le->pos = le->len;
}

int le_backspace_cp(LineEdit *le)
{
    if (!le || !le->buf || le->pos == 0)
        return 0;
    size_t start = utf8_prev_cp_start(le->buf, le->pos);
    size_t bytes = le->pos - start;
    if (bytes == 0)
        return 0;
    if (le->pos < le->len)
        memmove(le->buf + start, le->buf + le->pos, le->len - le->pos);
    le->len -= bytes;
    le->pos = start;
    le->buf[le->len] = '\0';
    return 1;
}

int le_delete_cp(LineEdit *le)
{
    if (!le || !le->buf || le->pos >= le->len)
        return 0;
    size_t end = utf8_next_cp_end(le->buf, le->pos);
    size_t bytes = end - le->pos;
    if (bytes == 0)
        return 0;
    if (end < le->len)
        memmove(le->buf + le->pos, le->buf + end, le->len - end);
    le->len -= bytes;
    le->buf[le->len] = '\0';
    return 1;
}

int le_insert_codepoint(LineEdit *le, int cp)
{
    if (!le)
        return 0;
    char utf8[5];
    int len = 0;
    if (cp < 0x80)
    {
        utf8[len++] = (char)cp;
    }
    else if (cp < 0x800)
    {
        utf8[len++] = (char)(0xC0 | (cp >> 6));
        utf8[len++] = (char)(0x80 | (cp & 0x3F));
    }
    else if (cp < 0x10000)
    {
        utf8[len++] = (char)(0xE0 | (cp >> 12));
        utf8[len++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        utf8[len++] = (char)(0x80 | (cp & 0x3F));
    }
    else
    {
        utf8[len++] = (char)(0xF0 | (cp >> 18));
        utf8[len++] = (char)(0x80 | ((cp >> 12) & 0x3F));
        utf8[len++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        utf8[len++] = (char)(0x80 | (cp & 0x3F));
    }
    utf8[len] = '\0';
    /* Insert bytes at current position */
    for (int i = 0; i < len; ++i)
    {
        if (!le_insert_char(le, (unsigned char)utf8[i]))
            return 0;
    }
    return 1;
}

/* Composition logic removed for simplicity; rely on platform input to provide composed codepoints. */
