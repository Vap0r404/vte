#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

#define MAX_LINES 65536
#define LINE_CAP 8192

typedef enum { MODE_NORMAL, MODE_INSERT, MODE_COMMAND } Mode;

typedef struct {
    char *lines[MAX_LINES];
    size_t count;
} Buffer;

static void buffer_init(Buffer *b) { b->count = 0; }
static void buffer_free(Buffer *b) { for (size_t i = 0; i < b->count; ++i) free(b->lines[i]); }

static int buffer_load(Buffer *b, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    char buf[LINE_CAP];
    while (fgets(buf, sizeof(buf), f)) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
        b->lines[b->count] = malloc(len + 1);
        strcpy(b->lines[b->count], buf);
        b->count++;
        if (b->count >= MAX_LINES) break;
    }
    if (b->count == 0) { b->lines[0] = malloc(1); b->lines[0][0] = '\0'; b->count = 1; }
    fclose(f);
    return 0;
}

static int buffer_save(Buffer *b, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    for (size_t i = 0; i < b->count; ++i) fprintf(f, "%s\n", b->lines[i]);
    fclose(f);
    return 0;
}

static void show_help(void) {
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
        "Command examples:",
        "  :w         - save (requires editor started with filename)",
        "  :w new.txt - save-as to new.txt",
        "  :q         - quit",
        "  :wq        - save and quit",
        "  :h or :help- show this help",
        "",
        "Build & run:",
        "  PowerShell: .\\build.ps1   (builds bin\\vte.exe)",
        "  Cmd: build.bat",
        "  Make: mingw32-make or make (if available)",
        "",
        "Press any key to return...",
        NULL
    };
    int rows, cols; getmaxyx(stdscr, rows, cols);
    clear();
    int r = 0;
    for (const char **p = help_lines; *p; ++p) {
        mvprintw(r++, 0, "%s", *p);
    }
    refresh();
    getch();
    clear();
}



static void draw_screen(Buffer *b, size_t cx, size_t cy, size_t rowoff, size_t coloff, Mode mode, const char *status) {
    int rows, cols; getmaxyx(stdscr, rows, cols);
    werase(stdscr);
    size_t max_display = rows - 2;
    for (size_t i = 0; i < max_display; ++i) {
        size_t lineno = rowoff + i;
        if (lineno >= b->count) break;
        const char *line = b->lines[lineno];
        int linelen = (int)strlen(line);
        if ((int)coloff < linelen) {
            const char *start = line + coloff;
            mvprintw((int)i, 0, "%s", start);
        }
    }
    move(rows - 2, 0); clrtoeol();
    char mstr[16]; strcpy(mstr, mode == MODE_INSERT ? "INSERT" : (mode == MODE_COMMAND ? "COMMAND" : "NORMAL"));
    mvprintw(rows - 2, 0, "-- %s -- %s", mstr, status ? status : "");
    move(rows - 1, 0); clrtoeol();
    refresh();
    int curs_y = (int)cy - (int)rowoff;
    int curs_x = (int)cx - (int)coloff;
    if (curs_y >= 0 && curs_y < (int)max_display && curs_x >= 0 && curs_x < cols) move(curs_y, curs_x);
}

int main(int argc, char **argv) {
    Buffer buf; buffer_init(&buf);
    const char *path = NULL; if (argc >= 2) path = argv[1];
    if (path) {
        if (buffer_load(&buf, path) != 0) { fprintf(stderr, "Could not open '%s' - starting empty\n", path); buf.lines[0] = malloc(1); buf.lines[0][0] = '\0'; buf.count = 1; }
    } else {
        buf.lines[0] = malloc(1); buf.lines[0][0] = '\0'; buf.count = 1;
    }

    size_t cx = 0, cy = 0, rowoff = 0, coloff = 0;
    Mode mode = MODE_NORMAL;
    char status[256] = "";

    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(1);

    int ch;
    while (1) {
        int rows, cols; getmaxyx(stdscr, rows, cols);
        int max_display = rows - 2;
        draw_screen(&buf, cx, cy, rowoff, coloff, mode, status);
        ch = getch();
        if (mode == MODE_NORMAL) {
            if (ch == ':') {
                mode = MODE_COMMAND;
                echo(); curs_set(1);
                mvprintw(rows - 1, 0, ":"); clrtoeol();
                char cmd[256]; move(rows - 1, 1); getnstr(cmd, 250);
                noecho();
                if (strcmp(cmd, "help") == 0 || strcmp(cmd, "h") == 0) {
                    show_help();
                } else if (strncmp(cmd, "w ", 2) == 0) {
                    char *fname = cmd + 2;
                    if (buffer_save(&buf, fname) == 0) snprintf(status, sizeof(status), "Saved to %s", fname); else snprintf(status, sizeof(status), "Save failed");
                } else if (strcmp(cmd, "w") == 0) {
                    if (path) { if (buffer_save(&buf, path) == 0) snprintf(status, sizeof(status), "Saved"); else snprintf(status, sizeof(status), "Save failed"); } else snprintf(status, sizeof(status), "No filename");
                } else if (strcmp(cmd, "q") == 0) break;
                else if (strcmp(cmd, "wq") == 0) { if (path) buffer_save(&buf, path); break; }
                else snprintf(status, sizeof(status), "Unknown: %s", cmd);
                mode = MODE_NORMAL;
                continue;
            }
            if (ch == 'i') { mode = MODE_INSERT; strcpy(status, ""); continue; }
            if (ch == KEY_LEFT || ch == 'h') { if (cx > 0) cx--; }
            else if (ch == KEY_RIGHT || ch == 'l') { if (cx < strlen(buf.lines[cy])) cx++; }
            else if (ch == KEY_UP || ch == 'k') { if (cy > 0) { cy--; if (cx > strlen(buf.lines[cy])) cx = strlen(buf.lines[cy]); if (cy < rowoff) rowoff = cy; } }
            else if (ch == KEY_DOWN || ch == 'j') { if (cy + 1 < buf.count) { cy++; if (cx > strlen(buf.lines[cy])) cx = strlen(buf.lines[cy]); if (cy >= rowoff + (size_t)max_display) rowoff = cy - max_display + 1; } }
        } else if (mode == MODE_INSERT) {
            if (ch == 27) { mode = MODE_NORMAL; continue; }
            if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                if (cx > 0) { char *line = buf.lines[cy]; size_t len = strlen(line); for (size_t i = cx - 1; i < len; ++i) line[i] = line[i+1]; cx--; line[len - 1] = '\0'; }
                else if (cx == 0 && cy > 0) { size_t prevlen = strlen(buf.lines[cy - 1]); size_t curlen = strlen(buf.lines[cy]); buf.lines[cy - 1] = realloc(buf.lines[cy - 1], prevlen + curlen + 1); strcat(buf.lines[cy - 1], buf.lines[cy]); free(buf.lines[cy]); for (size_t i = cy; i < buf.count - 1; ++i) buf.lines[i] = buf.lines[i + 1]; buf.count--; cy--; cx = prevlen; }
            } else if (ch == '\n' || ch == '\r') {
                char *line = buf.lines[cy]; size_t len = strlen(line); char *left = malloc(cx + 1); strncpy(left, line, cx); left[cx] = '\0'; char *right = malloc(len - cx + 1); strcpy(right, line + cx); free(buf.lines[cy]); buf.lines[cy] = left; if (buf.count + 1 < MAX_LINES) { for (size_t i = buf.count; i > cy + 1; --i) buf.lines[i] = buf.lines[i - 1]; buf.lines[cy + 1] = right; buf.count++; cy++; cx = 0; } else free(right);
            } else if (ch >= 32 && ch < 127) {
                char *line = buf.lines[cy]; size_t len = strlen(line); if (cx > len) cx = len; if (len + 1 >= LINE_CAP) continue; char *newline = realloc(line, len + 2); if (!newline) continue; for (size_t i = len; i > cx; --i) newline[i] = newline[i - 1]; newline[cx] = (char)ch; newline[len + 1] = '\0'; buf.lines[cy] = newline; cx++; if ((int)cx - (int)coloff >= cols) coloff = cx - cols + 1; }
            if (cy < rowoff) rowoff = cy; if (cy >= rowoff + (size_t)max_display) rowoff = cy - max_display + 1;
        }
    }

    endwin();
    buffer_free(&buf);
    return 0;
    }
