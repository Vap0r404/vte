#ifndef VTE_SYNTAX_H
#define VTE_SYNTAX_H

#include <curses.h>

void syntax_init(void);
void syntax_draw_line(const char *line, int row, int coloff, int cols);

#endif
