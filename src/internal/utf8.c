/* Prefer wide-char input to properly handle dead keys and composed characters when available */
#include "utf8.h"
#include <curses.h>
#if defined(PDC_WIDE) || defined(NCURSES_WIDECHAR) || defined(_XOPEN_SOURCE_EXTENDED)
#include <wchar.h>
#define VTE_HAVE_WIDE_INPUT 1
#endif
#ifdef _WIN32
#include <windows.h>
#endif

int utf8_getch(void)
{
#ifdef VTE_HAVE_WIDE_INPUT
    /* Try wide-character input first; returns composed characters properly */
    wint_t wch = 0;
    int rc = get_wch(&wch);
    if (rc == KEY_CODE_YES)
        return (int)wch;
    if (rc == OK)
        return (int)wch;
    /* else fall through to byte-wise path */
#endif
    /* Byte-wise getch decoding of UTF-8 or current Windows code page */
    int ch = getch();
    if (ch < 0 || ch >= KEY_MIN)
        return ch;
    if (ch < 128)
        return ch;
#ifdef _WIN32
    /* If console code page is not UTF-8, map single byte to Unicode via Windows */
    UINT cp = GetConsoleCP();
    if (cp != 65001)
    {
        wchar_t wch = 0;
        char mb = (char)ch;
        int n = MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, &mb, 1, &wch, 1);
        if (n == 1 && wch != 0)
            return (int)wch;
    }
#endif
    if ((ch & 0xE0) == 0xC0)
    {
        int ch2 = getch();
        if ((ch2 & 0xC0) != 0x80)
            return -1;
        return ((ch & 0x1F) << 6) | (ch2 & 0x3F);
    }
    else if ((ch & 0xF0) == 0xE0)
    {
        int ch2 = getch();
        int ch3 = getch();
        if ((ch2 & 0xC0) != 0x80 || (ch3 & 0xC0) != 0x80)
            return -1;
        return ((ch & 0x0F) << 12) | ((ch2 & 0x3F) << 6) | (ch3 & 0x3F);
    }
    else if ((ch & 0xF8) == 0xF0)
    {
        int ch2 = getch();
        int ch3 = getch();
        int ch4 = getch();
        if ((ch2 & 0xC0) != 0x80 || (ch3 & 0xC0) != 0x80 || (ch4 & 0xC0) != 0x80)
            return -1;
        return ((ch & 0x07) << 18) | ((ch2 & 0x3F) << 12) | ((ch3 & 0x3F) << 6) | (ch4 & 0x3F);
    }
    return ch;
}
