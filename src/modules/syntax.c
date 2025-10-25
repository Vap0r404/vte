#include "syntax.h"
#include <string.h>
#include <ctype.h>

/* Simple C-like syntax highlighter: keywords, strings, single-line comments */
static const char *keywords[] = {
    "int","char","long","short","float","double","return","if","else","for","while","do","switch","case","default","break","continue","struct","union","typedef","enum","static","const","void","unsigned","signed","extern","sizeof", NULL
};

void syntax_init(void)
{
    if (!has_colors()) return;
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
    if (!line) return;
    int col = 0;
    int i = (int)coloff;
    int n = (int)strlen(line);
    while (i < n && col < cols)
    {
        char c = line[i];
        if (c == '/' && i + 1 < n && line[i+1] == '/')
        {
            attron(COLOR_PAIR(3));
            mvaddnstr(row, col, line + i, cols - col);
            attroff(COLOR_PAIR(3));
            break;
        }
        else if (c == '"' || c == '\'')
        {
            char q = c;
            int j = i;
            do { ++j; } while (j < n && line[j] != q);
            int len = j - i + (j < n ? 1 : 0);
            attron(COLOR_PAIR(2));
            mvaddnstr(row, col, line + i, len > cols - col ? cols - col : len);
            attroff(COLOR_PAIR(2));
            col += len;
            i += len;
        }
        else if (isalpha((unsigned char)c) || c == '_')
        {
            int j = i+1;
            while (j < n && (isalnum((unsigned char)line[j]) || line[j] == '_')) ++j;
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
            ++col; ++i;
        }
    }
}
