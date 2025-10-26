#include "wrap.h"
#include <curses.h>
#include <string.h>
#include <stdint.h>

/* Minimal UTF-8 decoding and column width helpers */
static int utf8_decode_advance(const char *s, int *out_cp)
{
    const unsigned char *p = (const unsigned char *)s;
    if (*p == '\0')
        return 0;
    unsigned char b0 = p[0];
    if (b0 < 0x80)
    {
        if (out_cp)
            *out_cp = b0;
        return 1;
    }
    /* 2-byte */
    if ((b0 & 0xE0) == 0xC0 && (p[1] & 0xC0) == 0x80)
    {
        int cp = ((b0 & 0x1F) << 6) | (p[1] & 0x3F);
        if (out_cp)
            *out_cp = cp;
        return 2;
    }
    /* 3-byte */
    if ((b0 & 0xF0) == 0xE0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80)
    {
        int cp = ((b0 & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
        if (out_cp)
            *out_cp = cp;
        return 3;
    }
    /* 4-byte */
    if ((b0 & 0xF8) == 0xF0 && (p[1] & 0xC0) == 0x80 && (p[2] & 0xC0) == 0x80 && (p[3] & 0xC0) == 0x80)
    {
        int cp = ((b0 & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
        if (out_cp)
            *out_cp = cp;
        return 4;
    }
    /* invalid byte, treat as one */
    if (out_cp)
        *out_cp = b0;
    return 1;
}

static int ucs_display_width(int cp)
{
    /* Simple width: 0 for C0/C1 controls and combining diacritics; 1 otherwise.
       This treats East Asian wide codepoints as width 1, which is acceptable for Latin scripts. */
    if (cp == 0)
        return 0;
    if (cp < 32 || (cp >= 0x7F && cp < 0xA0))
        return 0;
    /* Combining Diacritical Marks (U+0300..U+036F) */
    if (cp >= 0x0300 && cp <= 0x036F)
        return 0;
    return 1;
}

int wrap_cols_for_prefix(const char *line, size_t byte_len)
{
    if (!line)
        return 0;
    int cols = 0;
    size_t i = 0;
    while (line[i] && i < byte_len)
    {
        int cp = 0;
        int adv = utf8_decode_advance(line + i, &cp);
        if (adv <= 0)
            break;
        if (i + (size_t)adv > byte_len)
            break; /* don't include partial cp */
        cols += ucs_display_width(cp);
        i += (size_t)adv;
    }
    return cols;
}

size_t wrap_byte_index_for_col(const char *line, int target_col)
{
    if (!line || target_col <= 0)
        return 0;
    int cols = 0;
    size_t i = 0;
    while (line[i])
    {
        int cp = 0;
        int adv = utf8_decode_advance(line + i, &cp);
        if (adv <= 0)
            break;
        int w = ucs_display_width(cp);
        if (cols + w > target_col)
            break;
        cols += w;
        i += (size_t)adv;
    }
    return i;
}

int wrap_calc_visual_lines(const char *line, int width)
{
    if (width <= 0)
        return 1;

    if (!line || !*line)
        return 1;
    /* compute total display columns */
    int total_cols = 0;
    size_t i = 0;
    while (line[i])
    {
        int cp = 0;
        int adv = utf8_decode_advance(line + i, &cp);
        if (adv <= 0)
            break;
        total_cols += ucs_display_width(cp);
        i += (size_t)adv;
    }
    if (total_cols <= 0)
        return 1;
    return (total_cols + width - 1) / width;
}

int wrap_draw_line(const char *line, int row, int col_start, int width, size_t coloff, int max_rows)
{
    if (!line || width <= 0)
        return 0;

    /* coloff is a column offset, not bytes */
    int start_col = (int)coloff;
    size_t line_len = strlen(line);
    int total_cols = wrap_cols_for_prefix(line, line_len); /* total columns */
    /* Ensure we visibly clear at least one row for empty lines or when starting beyond EOL */
    if (total_cols <= 0)
    {
        /* Clear one visual row for an empty line */
        mvprintw(row, col_start, "%-*s", width, "");
        return 1;
    }
    if (start_col >= total_cols)
    {
        /* View starts beyond end of line: draw a blank segment to clear */
        mvprintw(row, col_start, "%-*s", width, "");
        return 1;
    }

    int rows_used = 0;

    if (max_rows <= 0)
        max_rows = 1;

    int col_cursor = start_col;
    size_t byte_cursor = wrap_byte_index_for_col(line, col_cursor);
    while (col_cursor < total_cols && rows_used < max_rows)
    {
        int seg_cols = total_cols - col_cursor;
        if (seg_cols > width)
            seg_cols = width;
        size_t byte_end = wrap_byte_index_for_col(line, col_cursor + seg_cols);
        int bytes_to_print = (int)(byte_end - byte_cursor);

        /* Print exactly 'width' cells, padding with spaces, so we never overdraw the gutter
           and we overwrite any leftovers without using clrtoeol(). */
        mvprintw(row + rows_used, col_start, "%-*.*s", width, bytes_to_print, line + byte_cursor);

        byte_cursor = byte_end;
        col_cursor += seg_cols;
        rows_used++;
    }

    return rows_used > 0 ? rows_used : 1;
}
