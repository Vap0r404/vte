#include "syntax.h"
#include <string.h>
#include <ctype.h>

/* Simple C-like syntax highlighter: keywords, strings, single-line comments */
static const char *keywords[] = {
    "int", "char", "long", "short", "float", "double", "return", "if", "else", "for", "while", "do", "switch", "case", "default", "break", "continue", "struct", "union", "typedef", "enum", "static", "const", "void", "unsigned", "signed", "extern", "sizeof", NULL};

void syntax_init(void)
{
    if (!has_colors())
        return;
    start_color();
    use_default_colors();
    init_pair(1, COLOR_YELLOW, -1); /* keywords */
    init_pair(2, COLOR_GREEN, -1);  /* strings */
    init_pair(3, COLOR_CYAN, -1);   /* comments */
}

static int is_keyword(const char *s, size_t len)
{
    for (const char **k = keywords; *k; ++k)
    {
        if (strlen(*k) == len && strncmp(*k, s, len) == 0)
            return 1;
    }
    return 0;
}

void syntax_draw_line(const char *line, int row, int coloff, int cols)
{
    if (!line)
        return;
    int col = 0;
    int i = (int)coloff;
    int n = (int)strlen(line);
    while (i < n && col < cols)
    {
        char c = line[i];
        if (c == '/' && i + 1 < n && line[i + 1] == '/')
        {
            attron(COLOR_PAIR(3));
            int to_print = (int)strlen(line + i);
            if (to_print > cols - col)
                to_print = cols - col;
            if (to_print > 0)
                mvaddnstr(row, col, line + i, to_print);
            attroff(COLOR_PAIR(3));
            break;
        }
        else if (c == '/' && i + 1 < n && line[i + 1] == '*')
        {
            /* block comment on single line (slash-star ... star-slash) */
            int j = i + 2;
            while (j + 1 < n && !(line[j] == '*' && line[j + 1] == '/'))
                ++j;
            int len = (j + 1 < n) ? (j + 2 - i) : (j - i + 1);
            int to_print = len > cols - col ? cols - col : len;
            if (to_print > 0)
            {
                attron(COLOR_PAIR(3));
                mvaddnstr(row, col, line + i, to_print);
                attroff(COLOR_PAIR(3));
            }
            col += len;
            i += len;
            continue;
        }
        else if (c == '"' || c == '\'')
        {
            /* handle quoted strings/char literals and escaped quotes */
            char q = c;
            int j = i + 1;
            while (j < n)
            {
                if (line[j] == '\\' && j + 1 < n)
                    j += 2; /* skip escaped char */
                else if (line[j] == q)
                {
                    ++j; /* include closing quote */
                    break;
                }
                else
                    ++j;
            }
            int len = j - i;
            int to_print = len > cols - col ? cols - col : len;
            if (to_print > 0)
            {
                attron(COLOR_PAIR(2));
                mvaddnstr(row, col, line + i, to_print);
                attroff(COLOR_PAIR(2));
            }
            col += len;
            i += len;
        }
        else if (isalpha((unsigned char)c) || c == '_')
        {
            int j = i + 1;
            while (j < n && (isalnum((unsigned char)line[j]) || line[j] == '_'))
                ++j;
            int len = j - i;
            if (is_keyword(line + i, len))
            {
                attron(COLOR_PAIR(1));
                mvaddnstr(row, col, line + i, len > cols - col ? cols - col : len);
                attroff(COLOR_PAIR(1));
            }
            else
            {
                mvaddnstr(row, col, line + i, len > cols - col ? cols - col : len);
            }
            col += len;
            i += len;
        }
        else
        {
            mvaddnstr(row, col, line + i, 1);
            ++col;
            ++i;
        }
    }
}
