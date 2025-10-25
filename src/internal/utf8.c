#include "utf8.h"
#include <curses.h>

int utf8_getch(void)
{
    int ch = getch();

    /* Handle special keys and ASCII directly */
    if (ch < 0 || ch >= KEY_MIN)
        return ch;

    if (ch < 128)
        return ch;

    /* Multi-byte UTF-8 sequence */
    if ((ch & 0xE0) == 0xC0)
    {
        /* 2-byte sequence: 110xxxxx 10xxxxxx */
        int ch2 = getch();
        if ((ch2 & 0xC0) != 0x80)
            return -1;
        return ((ch & 0x1F) << 6) | (ch2 & 0x3F);
    }
    else if ((ch & 0xF0) == 0xE0)
    {
        /* 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx */
        int ch2 = getch();
        int ch3 = getch();
        if ((ch2 & 0xC0) != 0x80 || (ch3 & 0xC0) != 0x80)
            return -1;
        return ((ch & 0x0F) << 12) | ((ch2 & 0x3F) << 6) | (ch3 & 0x3F);
    }
    else if ((ch & 0xF8) == 0xF0)
    {
        /* 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        int ch2 = getch();
        int ch3 = getch();
        int ch4 = getch();
        if ((ch2 & 0xC0) != 0x80 || (ch3 & 0xC0) != 0x80 || (ch4 & 0xC0) != 0x80)
            return -1;
        return ((ch & 0x07) << 18) | ((ch2 & 0x3F) << 12) | ((ch3 & 0x3F) << 6) | (ch4 & 0x3F);
    }

    /* Invalid or single byte > 127 */
    return ch;
}
