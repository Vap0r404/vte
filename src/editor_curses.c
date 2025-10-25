#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "modules/line_edit.h"
#include "config.h"

#define MAX_LINES 65536
#define LINE_CAP 8192
typedef enum
{
    MODE_NORMAL,
    MODE_INSERT,
    MODE_COMMAND,
    MODE_SEARCH
} Mode;

#include "modules/buffer.h"
#include "modules/syntax.h"
#include "modules/navigation.h"
#include "modules/status.h"
#include "internal/resize.h"
#include "internal/mouse.h"
#include "internal/wrap.h"
#include "internal/utf8.h"
#include "internal/wrap_cache.h"

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
        "Mouse:",
        "  Single-click to move the cursor (works in NORMAL and INSERT modes)",
        "  Click mapping respects visual wrapping (long lines wrap on screen)",
        "",
        "Normal mode keys:",
        "  h/j/k/l    - left/down/up/right",
        "  Arrow keys - work as well",
        "  i          - enter INSERT mode",
        "  :          - enter COMMAND mode",
        "  /          - pattern search mode",
        "  n          - find next match (forward)",
        "  N          - find previous match (backward)",
        "",
        "Buffers:",
        "  vte supports multiple buffers (up to 16 files open at once).",
        "  The status line shows buffer index [N/Total], filename, and line:col.",
        "  Each buffer tracks its own content, path, and modified state.",
        "  Use :e to open files and :bn/:bp to switch between buffers.",
        "",
        "Command examples:",
        "  :w         - save current buffer (requires filename)",
        "  :w new.txt - save-as current buffer to new.txt",
        "  :e filename - open/switch to file in a buffer",
        "  :bn        - switch to next buffer",
        "  :bp        - switch to previous buffer",
        "  :123       - goto line 123 (any number)",
        "  /pattern   - search forward for 'pattern'",
        "  :set       - show current settings",
        "  :set name=value - change a setting",
        "  :q         - quit (all buffers)",
        "  :wq        - save current buffer and quit",
        "  :h or :help- show this help",
        "",
        "Build & run:",
        "  PowerShell: .\\build.ps1   (builds bin\\vte.exe)",
        "  Cmd: build.bat",
        "  Make: mingw32-make or make (if available)",
        "",
        "Navigation: j/k or arrow keys to scroll, q or any other key to return",
        NULL};

    int total_lines = 0;
    for (const char **p = help_lines; *p; ++p)
        total_lines++;

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    (void)cols; /* suppress unused warning */
    int offset = 0;
    int max_offset = total_lines - rows;
    if (max_offset < 0)
        max_offset = 0;

    while (1)
    {
        clear();
        for (int r = 0; r < rows && offset + r < total_lines; ++r)
        {
            mvprintw(r, 0, "%s", help_lines[offset + r]);
        }
        refresh();

        int ch = getch();
        if (ch == 'j' || ch == KEY_DOWN)
        {
            if (offset < max_offset)
                offset++;
        }
        else if (ch == 'k' || ch == KEY_UP)
        {
            if (offset > 0)
                offset--;
        }
        else if (ch == 'q' || ch == 27)
        {
            break;
        }
        else
        {
            break; /* any other key exits */
        }
    }
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

static void draw_screen(Buffer *b, size_t cx, size_t cy, size_t rowoff, size_t coloff, Mode mode, const char *status, LineEdit *le, int le_active, int line_num_width, WrapCache *wc)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    /* Hide cursor during redraw to avoid flicker */
    curs_set(0);
    size_t max_display = rows - 2;

    /* Available width for text excluding line numbers */
    int text_width = cols - line_num_width;
    if (text_width < 1)
        text_width = 1;

    /* Determine which buffer line starts at the top, based on visual row offset */
    size_t start_line = 0;
    size_t skip_rows_in_first = 0;
    size_t remain = rowoff;
    for (size_t i = 0; i < b->count; ++i)
    {
        const char *ln = b->lines[i];
        int vis = wrap_cache_get(wc, ln, i);
        if ((size_t)vis > remain)
        {
            start_line = i;
            skip_rows_in_first = remain;
            break;
        }
        remain -= (size_t)vis;
        if (i + 1 == b->count)
        {
            start_line = i;
            skip_rows_in_first = 0;
        }
    }

    /* Draw buffer lines with wrapping starting from the computed offset */
    size_t screen_row = 0;
    for (size_t lineno = start_line; lineno < b->count && screen_row < max_display; ++lineno)
    {
        const char *line = b->lines[lineno];
        if (mode == MODE_INSERT && le_active && le && lineno == cy && le->buf)
            line = le->buf;

        /* First visual row: draw line number */
        mvprintw((int)screen_row, 0, "%*zu ", line_num_width - 1, lineno + 1);

        int max_rows_for_line = (int)(max_display - screen_row);
        size_t line_start_coloff = coloff;
        if (lineno == start_line && skip_rows_in_first > 0)
            line_start_coloff += skip_rows_in_first * (size_t)text_width;

        int used = wrap_draw_line(line, (int)screen_row, line_num_width, text_width, line_start_coloff, max_rows_for_line);
        if (used < 1)
            used = 1;

        for (int k = 1; k < used && screen_row + (size_t)k < max_display; ++k)
        {
            mvprintw((int)(screen_row + k), 0, "%*s ", line_num_width - 1, "");
        }

        screen_row += (size_t)used;
    }

    /* Clear remaining rows below the last drawn content without erasing the whole screen */
    for (; screen_row < max_display; ++screen_row)
    {
        /* Blank gutter and text area for each leftover row */
        mvprintw((int)screen_row, 0, "%*s ", line_num_width - 1, "");
        /* Print padded empty segment to ensure text area is cleared */
        mvprintw((int)screen_row, line_num_width, "%-*s", cols - line_num_width, "");
    }
    move(rows - 2, 0);
    clrtoeol();

    /* Use status module to format status line */
    char mstr[16];
    strcpy(mstr, mode == MODE_INSERT ? "INSERT" : (mode == MODE_COMMAND ? "COMMAND" : (mode == MODE_SEARCH ? "SEARCH" : "NORMAL")));
    char status_line[512];
    status_format(status_line, sizeof(status_line), mstr, b->path,
                  buffer_index(), buffer_count(), b->dirty, cy, cx, cols);

    mvprintw(rows - 2, 0, "%s", status_line);

    /* Show status message if present */
    if (status && status[0])
    {
        mvprintw(rows - 2, strlen(mstr) + strlen(b->path ? b->path : "[No file]") + 20, "  %s", status);
    }

    move(rows - 1, 0);
    clrtoeol();

    /* Position cursor accounting for wrapping and line number column */
    int vcursor = 0;
    for (size_t i = 0; i < cy; ++i)
    {
        const char *ln = b->lines[i];
        int vis = wrap_cache_get(wc, ln, i);
        vcursor += vis;
    }
    vcursor += ((int)cx - (int)coloff) / text_width;

    int curs_y = vcursor - (int)rowoff;
    int rel_x = (int)cx - (int)coloff;
    if (rel_x < 0)
        rel_x = 0;
    int curs_x = (rel_x % text_width) + line_num_width;

    /* Clamp cursor into visible area */
    if (curs_y >= 0 && curs_y < (int)max_display && curs_x >= 0 && curs_x < cols)
        move(curs_y, curs_x);

    /* Batch updates for smoother rendering and apply after final cursor move */
    wnoutrefresh(stdscr);
    doupdate();

    /* Restore cursor visibility after redraw */
    curs_set(1);
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
    /* wrap cache for current buffer */
    WrapCache wc;
    wrap_cache_init(&wc, buf->count);
    size_t cx = 0, cy = 0, rowoff = 0, coloff = 0;
    Mode mode = MODE_NORMAL;
    char status[256] = "";

    /* config state */
    EditorConfig config;
    config_init(&config);
    /* Try to load .vterc from current directory, generate if not found */
    if (config_load(&config, ".vterc") != 0)
    {
        config_generate(".vterc");
        config_load(&config, ".vterc");
    }

    /* navigation state */
    NavState nav;
    nav_init(&nav);

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
    mouse_init();

    int ch;
    while (1)
    {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        int max_display = rows - 2;

        /* Calculate line number width */
        nav.line_num_width = nav_calc_line_num_width(buf->count);

        /* Update wrap cache width and size for this frame */
        wrap_cache_set_width(&wc, cols - nav.line_num_width < 1 ? 1 : cols - nav.line_num_width);
        wrap_cache_ensure(&wc, buf->count);

        draw_screen(buf, cx, cy, rowoff, coloff, mode, status, &le, le_active, nav.line_num_width, &wc);
        ch = utf8_getch();

        if (ch == KEY_RESIZE)
        {
            handle_resize();
            /* Invalidate wrap cache on resize */
            wrap_cache_invalidate_all(&wc);
            continue;
        }

        if (ch == KEY_MOUSE)
        {
            int text_width = cols - nav.line_num_width;
            if (text_width < 1)
                text_width = 1;

            /* In INSERT mode, commit current line edits before moving the cursor/line. */
            if (mode == MODE_INSERT && le_active)
            {
                char *committed = le_take_string(&le);
                if (committed)
                {
                    free(buf->lines[cy]);
                    buf->lines[cy] = committed;
                    buf->dirty = 1;
                }
                le_free(&le);
                le_active = 0;
            }

            if (mouse_handle_click(&cx, &cy, &rowoff, buf->lines, buf->count, nav.line_num_width, max_display, text_width))
            {
                snprintf(status, sizeof(status), "Click: line %zu, col %zu", cy + 1, cx + 1);
                /* If in INSERT mode, (re)initialize line editor at new position */
                if (mode == MODE_INSERT)
                {
                    le_init(&le, buf->lines[cy]);
                    le.pos = (cx < le.len) ? cx : le.len;
                    le_active = 1;
                    /* clicked line may change wrapping on edit later, no action now */
                }
            }
            continue;
        }

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
                        /* reset wrap cache for new buffer */
                        wrap_cache_free(&wc);
                        wrap_cache_init(&wc, buf->count);
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
                    wrap_cache_free(&wc);
                    wrap_cache_init(&wc, buf->count);
                }
                else if (strcmp(cmd, "bp") == 0)
                {
                    buffer_prev();
                    buf = buffer_current();
                    cx = cy = rowoff = coloff = 0;
                    snprintf(status, sizeof(status), "Buffer %d/%zu", buffer_index() + 1, buffer_count());
                    wrap_cache_free(&wc);
                    wrap_cache_init(&wc, buf->count);
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
                else if (cmd[0] >= '0' && cmd[0] <= '9')
                {
                    /* :number - goto line */
                    size_t target_line = (size_t)atoi(cmd);
                    if (nav_goto_line(target_line, &cy, &cx, &rowoff, max_display, buf->count))
                        snprintf(status, sizeof(status), "Line %zu", target_line);
                    else
                        snprintf(status, sizeof(status), "Invalid line: %s", cmd);
                }
                else if (strcmp(cmd, "set") == 0)
                {
                    /* :set - show current settings */
                    config_show(&config, status, sizeof(status));
                }
                else if (strncmp(cmd, "set ", 4) == 0)
                {
                    /* :set name=value */
                    config_set(&config, cmd + 4, status, sizeof(status));
                }
                else
                    snprintf(status, sizeof(status), "Unknown: %s", cmd);
                mode = MODE_NORMAL;
                continue;
            }
            if (ch == '/')
            {
                mode = MODE_SEARCH;
                curs_set(1);
                mvprintw(rows - 1, 0, "/");
                clrtoeol();
                char pattern[256];
                move(rows - 1, 1);
                get_command_line(rows - 1, pattern, 250);

                if (pattern[0] != '\0')
                {
                    int result = nav_search_forward(pattern, buf, &cy, &cx, &rowoff, max_display, &nav);
                    if (result == 1)
                        snprintf(status, sizeof(status), "/%s", pattern);
                    else if (result == 2)
                        snprintf(status, sizeof(status), "/%s (wrapped)", pattern);
                    else
                        snprintf(status, sizeof(status), "Pattern not found: %s", pattern);
                }

                mode = MODE_NORMAL;
                continue;
            }
            if (ch == 'n')
            {
                if (nav.last_search[0] != '\0')
                {
                    int result = nav_search_next(buf, &cy, &cx, &rowoff, max_display, &nav);
                    if (result == 1)
                        snprintf(status, sizeof(status), "/%s", nav.last_search);
                    else if (result == 2)
                        snprintf(status, sizeof(status), "/%s (wrapped)", nav.last_search);
                    else
                        snprintf(status, sizeof(status), "Pattern not found: %s", nav.last_search);
                }
                else
                    snprintf(status, sizeof(status), "No previous search");
                continue;
            }
            if (ch == 'N')
            {
                if (nav.last_search[0] != '\0')
                {
                    int result = nav_search_prev(buf, &cy, &cx, &rowoff, max_display, &nav);
                    if (result == 1)
                        snprintf(status, sizeof(status), "?%s", nav.last_search);
                    else if (result == 2)
                        snprintf(status, sizeof(status), "?%s (wrapped)", nav.last_search);
                    else
                        snprintf(status, sizeof(status), "Pattern not found: %s", nav.last_search);
                }
                else
                    snprintf(status, sizeof(status), "No previous search");
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
                    wrap_cache_invalidate_line(&wc, cy);
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
            else if (ch == KEY_UP)
            {
                if (cy > 0)
                {
                    /* Save current line edit */
                    char *tmp = le_take_string(&le);
                    if (tmp)
                    {
                        free(buf->lines[cy]);
                        buf->lines[cy] = tmp;
                        /* Current line content changed due to take_string; invalidate */
                        wrap_cache_invalidate_line(&wc, cy);
                    }
                    cy--;
                    /* Re-init with new line */
                    le_free(&le);
                    le_init(&le, buf->lines[cy]);
                    /* Try to preserve column position */
                    if (cx > le.len)
                        le.pos = le.len;
                    else
                        le.pos = cx;
                    cx = le.pos;
                }
            }
            else if (ch == KEY_DOWN)
            {
                if (cy < buf->count - 1)
                {
                    /* Save current line edit */
                    char *tmp = le_take_string(&le);
                    if (tmp)
                    {
                        free(buf->lines[cy]);
                        buf->lines[cy] = tmp;
                    }
                    cy++;
                    /* Re-init with new line */
                    le_free(&le);
                    le_init(&le, buf->lines[cy]);
                    /* Try to preserve column position */
                    if (cx > le.len)
                        le.pos = le.len;
                    else
                        le.pos = cx;
                    cx = le.pos;
                    /* Adjust scroll if needed */
                    if (cy >= rowoff + (size_t)max_display)
                        rowoff = cy - max_display + 1;
                }
            }
            else if (ch == KEY_HOME)
            {
                le_move_home(&le);
            }
            else if (ch == KEY_END)
            {
                le_move_end(&le);
            }
            else if (ch == KEY_DC)
            {
                if (le_delete(&le))
                    buf->dirty = 1;
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
                            /* Buffer structure changed; reset wrap cache */
                            wrap_cache_ensure(&wc, buf->count);
                            wrap_cache_invalidate_all(&wc);
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
                        /* Buffer grew; reset cache size and invalidate */
                        wrap_cache_ensure(&wc, buf->count);
                        wrap_cache_invalidate_all(&wc);
                    }
                    else
                        free(right);
                }
            }
            else if (ch >= 32 && ch < 127)
            {
                if (le_insert_char(&le, ch))
                {
                    buf->dirty = 1;
                    wrap_cache_invalidate_line(&wc, cy);
                }
            }
            else if (ch >= 128 && ch <= 0x10FFFF)
            {
                /* UTF-8 multi-byte character: encode and insert */
                char utf8[5];
                int len = 0;
                if (ch < 0x80)
                {
                    utf8[len++] = ch;
                }
                else if (ch < 0x800)
                {
                    utf8[len++] = 0xC0 | (ch >> 6);
                    utf8[len++] = 0x80 | (ch & 0x3F);
                }
                else if (ch < 0x10000)
                {
                    utf8[len++] = 0xE0 | (ch >> 12);
                    utf8[len++] = 0x80 | ((ch >> 6) & 0x3F);
                    utf8[len++] = 0x80 | (ch & 0x3F);
                }
                else
                {
                    utf8[len++] = 0xF0 | (ch >> 18);
                    utf8[len++] = 0x80 | ((ch >> 12) & 0x3F);
                    utf8[len++] = 0x80 | ((ch >> 6) & 0x3F);
                    utf8[len++] = 0x80 | (ch & 0x3F);
                }
                utf8[len] = '\0';
                /* Insert each byte */
                for (int i = 0; i < len; i++)
                {
                    if (le_insert_char(&le, (unsigned char)utf8[i]))
                    {
                        buf->dirty = 1;
                        wrap_cache_invalidate_line(&wc, cy);
                    }
                }
            }

            cx = le.pos;
            /* Wrapping enabled: disable horizontal scrolling */
            coloff = 0;

            /* Ensure cursor is visible by adjusting visual row offset */
            int text_width = cols - nav.line_num_width;
            if (text_width < 1)
                text_width = 1;
            int vcursor = 0;
            for (size_t i = 0; i < cy; ++i)
                vcursor += wrap_calc_visual_lines(buf->lines[i], text_width);
            vcursor += (int)cx / text_width;
            if (vcursor < (int)rowoff)
                rowoff = (size_t)vcursor;
            else if (vcursor >= (int)(rowoff + max_display))
                rowoff = (size_t)(vcursor - max_display + 1);
        }
    }

    endwin();
    buffer_free_all();
    return 0;
}
