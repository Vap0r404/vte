#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "modules/line_edit.h"

#define MAX_LINES 65536
#define LINE_CAP 8192
typedef enum
{
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND
} Mode;

#include "modules/buffer.h"
#include "modules/syntax.h"

static void show_help(void)
{
    const char *help_lines[] = {
        "vte - minimal vim-like editor (keys)",
        "",
        "Modes:",
        "  NORMAL - navigate and enter commands/insert",
        "  INSERT - type text (press Esc to return to NORMAL)",
        "  COMMAND - press : to enter, supports :w, :w filename, :q, :wq, :h, :help",
        "",
        "Normal mode keys:",
        "  h/j/k/l    - left/down/up/right",
        "  Arrow keys - work as well",
        "  i          - enter INSERT mode",
        "  :          - enter COMMAND mode",
        "",
        "Buffers:",
        "  vte supports multiple buffers (up to 16 files open at once).",
        "  The status line shows buffer index [N/Total] and filename.",
        "  Each buffer tracks its own content, path, and modified state.",
        "  Use :e to open files and :bn/:bp to switch between buffers.",
        "",
        "Command examples:",
        "  :w         - save current buffer (requires filename)",
        "  :w new.txt - save-as current buffer to new.txt",
        "  :e filename - open/switch to file in a buffer",
        "  :bn        - switch to next buffer",
        "  :bp        - switch to previous buffer",
        "  :q         - quit (all buffers)",
        "  :wq        - save current buffer and quit",
        "  :h or :help- show this help",
        "",
        "Build & run:",
        "  PowerShell: .\\build.ps1   (builds bin\\vte.exe)",
        "  Cmd: build.bat",
        "  Make: mingw32-make or make (if available)",
        "",
        "Press any key to return...",
        NULL};
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    (void)rows;
    (void)cols; /* suppress unused-var warnings: we only print static help */
    clear();
    int r = 0;
    for (const char **p = help_lines; *p; ++p)
    {
        mvprintw(r++, 0, "%s", *p);
    }
    refresh();
    getch();
    clear();
}

static void get_command_line(int row, char *out, int maxlen)
{
    int pos = 0, len = 0;
    out[0] = '\0';
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    (void)rows;
    (void)cols;
    while (1)
    {
        int c = getch();
        if (c == '\r' || c == '\n')
            break;
        else if (c == KEY_BACKSPACE || c == 127 || c == 8)
        {
            if (pos > 0)
            {
                for (int i = pos - 1; i < len - 1; ++i)
                    out[i] = out[i + 1];
                pos--;
                len--;
                out[len] = '\0';
            }
        }
        else if (c >= 32 && c < 127)
        {
            if (len + 1 < maxlen)
            {
                for (int i = len; i > pos; --i)
                    out[i] = out[i - 1];
                out[pos] = (char)c;
                pos++;
                len++;
                out[len] = '\0';
            }
        }
        mvprintw(row, 1, "%s", out);
        clrtoeol();
        move(row, 1 + pos);
        refresh();
    }
}

static void draw_screen(Buffer *b, size_t cx, size_t cy, size_t rowoff, size_t coloff, Mode mode, const char *status, LineEdit *le, int le_active)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    werase(stdscr);
    size_t max_display = rows - 2;
    for (size_t i = 0; i < max_display; ++i)
    {
        size_t lineno = rowoff + i;
        if (lineno >= b->count)
            break;
        const char *line = b->lines[lineno];
        /* If we're in INSERT mode and editing this line with a LineEdit, show the in-progress buffer */
        if (mode == MODE_INSERT && le_active && le && lineno == cy && le->buf)
            line = le->buf;
        int linelen = (int)strlen(line);
        if ((int)coloff < linelen)
        {
            const char *start = line + coloff;
            mvprintw((int)i, 0, "%s", start);
        }
    }
    move(rows - 2, 0);
    clrtoeol();
    char mstr[16];
    strcpy(mstr, mode == MODE_INSERT ? "INSERT" : (mode == MODE_COMMAND ? "COMMAND" : "NORMAL"));
    /* show filename, buffer index and modified flag */
    const char *fname = b->path && b->path[0] ? b->path : "[No file]";
    char fbuf[256];
    int idx = buffer_index();
    size_t total = buffer_count();
    if (b->dirty)
        snprintf(fbuf, sizeof(fbuf), "%s [%d/%zu] [+]", fname, idx + 1, total);
    else
        snprintf(fbuf, sizeof(fbuf), "%s [%d/%zu]", fname, idx + 1, total);
    mvprintw(rows - 2, 0, "-- %s -- %s  %s", mstr, fbuf, status ? status : "");
    move(rows - 1, 0);
    clrtoeol();
    refresh();
    int curs_y = (int)cy - (int)rowoff;
    int curs_x = (int)cx - (int)coloff;
    if (curs_y >= 0 && curs_y < (int)max_display && curs_x >= 0 && curs_x < cols)
        move(curs_y, curs_x);
}

int main(int argc, char **argv)
{
    /* initialize buffer pool and set current buffer */
    buffer_pool_init();
    if (argc >= 2)
    {
        if (buffer_open_file(argv[1]) < 0)
        {
            Buffer *buf = buffer_current();
            fprintf(stderr, "Could not open '%s' - starting empty\n", argv[1]);
            buf->lines[0] = malloc(1);
            buf->lines[0][0] = '\0';
            buf->count = 1;
            if (buf->path)
            {
                free(buf->path);
                buf->path = NULL;
            }
            buf->dirty = 0;
        }
    }
    else
    {
        Buffer *buf = buffer_current();
        buf->lines[0] = malloc(1);
        buf->lines[0][0] = '\0';
        buf->count = 1;
        buf->path = NULL;
        buf->dirty = 0;
    }

    /* pointer to currently active buffer */
    Buffer *buf = buffer_current();
    size_t cx = 0, cy = 0, rowoff = 0, coloff = 0;
    Mode mode = MODE_NORMAL;
    char status[256] = "";

    /* line editor state used only in INSERT mode */
    LineEdit le;
    le.buf = NULL;
    le.len = le.cap = le.pos = 0;
    int le_active = 0;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(1);
    syntax_init();

    int ch;
    while (1)
    {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        int max_display = rows - 2;
        draw_screen(buf, cx, cy, rowoff, coloff, mode, status, &le, le_active);
        ch = getch();
        if (mode == MODE_NORMAL)
        {
            if (ch == ':')
            {
                mode = MODE_COMMAND;
                curs_set(1);
                mvprintw(rows - 1, 0, ":");
                clrtoeol();
                char cmd[256];
                move(rows - 1, 1);
                get_command_line(rows - 1, cmd, 250);
                if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0)
                {
                    show_help();
                }
                else if (strncmp(cmd, "w ", 2) == 0)
                {
                    char *fname = cmd + 2;
                    /* clamp filename length when assembling status text to avoid snprintf truncation warning */
                    char _fname_s[240] = "";
                    if (fname && fname[0])
                    {
                        /* use snprintf to safely truncate and avoid strncpy truncation warnings */
                        snprintf(_fname_s, sizeof(_fname_s), "%.*s", (int)(sizeof(_fname_s) - 1), fname);
                        _fname_s[sizeof(_fname_s) - 1] = '\0';
                    }
                    if (buffer_save_current(fname) == 0)
                    {
                        /* update buffer path to the saved filename */
                        if (buf->path)
                            free(buf->path);
                        buf->path = strdup(fname);
                        snprintf(status, sizeof(status), "Saved to %s", _fname_s);
                        buf->dirty = 0;
                    }
                    else
                        snprintf(status, sizeof(status), "Save failed");
                }
                else if (strncmp(cmd, "e ", 2) == 0)
                {
                    char *fname = cmd + 2;
                    if (buffer_open_file(fname) >= 0)
                    {
                        buf = buffer_current();
                        cx = cy = rowoff = coloff = 0;
                        snprintf(status, sizeof(status), "Opened %s", fname);
                    }
                    else
                        snprintf(status, sizeof(status), "Open failed: %s", fname);
                }
                else if (strcmp(cmd, "bn") == 0)
                {
                    buffer_next();
                    buf = buffer_current();
                    cx = cy = rowoff = coloff = 0;
                    snprintf(status, sizeof(status), "Buffer %d/%zu", buffer_index() + 1, buffer_count());
                }
                else if (strcmp(cmd, "bp") == 0)
                {
                    buffer_prev();
                    buf = buffer_current();
                    cx = cy = rowoff = coloff = 0;
                    snprintf(status, sizeof(status), "Buffer %d/%zu", buffer_index() + 1, buffer_count());
                }
                else if (strcmp(cmd, "w") == 0)
                {
                    if (buf->path)
                    {
                        if (buffer_save_current(buf->path) == 0)
                        {
                            snprintf(status, sizeof(status), "Saved");
                            buf->dirty = 0;
                        }
                        else
                            snprintf(status, sizeof(status), "Save failed");
                    }
                    else
                        snprintf(status, sizeof(status), "No filename");
                }
                else if (strcmp(cmd, "q") == 0)
                    break;
                else if (strcmp(cmd, "wq") == 0)
                {
                    if (buf->path)
                    {
                        if (buffer_save_current(buf->path) == 0)
                            buf->dirty = 0;
                    }
                    break;
                }
                else
                    snprintf(status, sizeof(status), "Unknown: %s", cmd);
                mode = MODE_NORMAL;
                continue;
            }
            if (ch == 'i')
            {
                mode = MODE_INSERT;
                strcpy(status, "");
                continue;
            }
            if (ch == KEY_LEFT || ch == 'h')
            {
                if (cx > 0)
                    cx--;
            }
            else if (ch == KEY_RIGHT || ch == 'l')
            {
                if (cx < strlen(buf->lines[cy]))
                    cx++;
            }
            else if (ch == KEY_UP || ch == 'k')
            {
                if (cy > 0)
                {
                    cy--;
                    if (cx > strlen(buf->lines[cy]))
                        cx = strlen(buf->lines[cy]);
                    if (cy < rowoff)
                    {
                        rowoff = cy;
                    }
                }
            }
            else if (ch == KEY_DOWN || ch == 'j')
            {
                if (cy + 1 < buf->count)
                {
                    cy++;
                    if (cx > strlen(buf->lines[cy]))
                        cx = strlen(buf->lines[cy]);
                    if (cy >= rowoff + (size_t)max_display)
                    {
                        rowoff = cy - max_display + 1;
                    }
                }
            }
        }
        else if (mode == MODE_INSERT)
        {
            /* initialize line editor for the current line when first entering INSERT */
            if (!le_active)
            {
                le_init(&le, buf->lines[cy]);
                le.pos = cx;
                le_active = 1;
            }

            if (ch == 27)
            {
                /* exit insert mode: write back the current line */
                char *newline = le_take_string(&le);
                if (newline)
                {
                    free(buf->lines[cy]);
                    buf->lines[cy] = newline;
                    buf->dirty = 1;
                }
                le_free(&le);
                le_active = 0;
                mode = MODE_NORMAL;
                continue;
            }

            if (ch == KEY_LEFT)
            {
                le_move_left(&le);
            }
            else if (ch == KEY_RIGHT)
            {
                le_move_right(&le);
            }
            else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8)
            {
                if (le_backspace(&le))
                {
                    buf->dirty = 1;
                }
                else
                {
                    /* at column 0: join with previous line if possible */
                    if (le.pos == 0 && cy > 0)
                    {
                        size_t prevlen = strlen(buf->lines[cy - 1]);
                        char *joined = realloc(buf->lines[cy - 1], prevlen + le.len + 1);
                        if (joined)
                        {
                            memcpy(joined + prevlen, le.buf, le.len + 1);
                            buf->lines[cy - 1] = joined;
                            free(buf->lines[cy]);
                            for (size_t i = cy; i < buf->count - 1; ++i)
                                buf->lines[i] = buf->lines[i + 1];
                            buf->count--;
                            cy--;
                            /* reinit line editor to the end of the joined line */
                            le_free(&le);
                            le_init(&le, buf->lines[cy]);
                            le.pos = prevlen;
                            buf->dirty = 1;
                        }
                    }
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                /* split the current line at cursor */
                char *right = le_split(&le);
                char *left = le_take_string(&le);
                if (left)
                {
                    free(buf->lines[cy]);
                    buf->lines[cy] = left;
                }
                if (right)
                {
                    if (buf->count + 1 < MAX_LINES)
                    {
                        for (size_t i = buf->count; i > cy + 1; --i)
                            buf->lines[i] = buf->lines[i - 1];
                        buf->lines[cy + 1] = right;
                        buf->count++;
                        cy++;
                        le_free(&le);
                        le_init(&le, buf->lines[cy]);
                        le.pos = 0;
                        buf->dirty = 1;
                    }
                    else
                        free(right);
                }
            }
            else if (ch >= 32 && ch < 127)
            {
                if (le_insert_char(&le, ch))
                    buf->dirty = 1;
            }

            cx = le.pos;
            if ((int)cx - (int)coloff >= cols)
                coloff = cx - cols + 1;
            if (cy < rowoff)
            {
                rowoff = cy;
            }
            if (cy >= rowoff + (size_t)max_display)
            {
                rowoff = cy - max_display + 1;
            }
        }
    }

    endwin();
    buffer_free_all();
    return 0;
}
