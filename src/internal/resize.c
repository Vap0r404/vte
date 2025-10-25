#include "resize.h"
#include <curses.h>

void handle_resize(void)
{
    resize_term(0, 0);
    clearok(curscr, TRUE);
    clear();
    refresh();
}
