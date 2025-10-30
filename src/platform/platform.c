#include "platform.h"

#ifdef _WIN32
#include <windows.h>

void platform_init(void)
{
    /* For wide-curses builds, input/output go through wide APIs, so no need to force code pages.
       For narrow builds, forcing UTF-8 helps when decoding multibyte sequences. */
#ifndef PDC_WIDE
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
#endif
}

void platform_cleanup(void)
{
    /* No cleanup needed on Windows currently */
}

#else
/* Unix/Linux/macOS */

void platform_init(void)
{
    /* ncurses handles UTF-8 via locale; nothing extra needed */
}

void platform_cleanup(void)
{
    /* No cleanup needed on Unix currently */
}

#endif
