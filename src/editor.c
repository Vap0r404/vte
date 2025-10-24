#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX_LINES 65536
#define LINE_CAP 4096

typedef enum { MODE_NORMAL, MODE_INSERT, MODE_COMMAND } Mode;

typedef struct {
    char *lines[MAX_LINES];
    size_t count;
} Buffer;

static void buffer_init(Buffer *b) {
    b->count = 0;
}

static void buffer_free(Buffer *b) {
    for (size_t i = 0; i < b->count; ++i) free(b->lines[i]);
}

static int buffer_load(Buffer *b, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    char buf[LINE_CAP];
    while (fgets(buf, sizeof(buf), f)) {
        size_t len = strlen(buf);
        // strip CRLF
        while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r')) { buf[--len] = '\0'; }
        b->lines[b->count] = (char*)malloc(len + 1);
        strcpy(b->lines[b->count], buf);
        b->count++;
        if (b->count >= MAX_LINES) break;
    }
    if (b->count == 0) {
        b->lines[0] = (char*)malloc(1); b->lines[0][0] = '\0'; b->count = 1;
    }
    fclose(f);
    return 0;
}

static int buffer_save(Buffer *b, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    for (size_t i = 0; i < b->count; ++i) {
        fprintf(f, "%s\n", b->lines[i]);
    }
    fclose(f);
    return 0;
}

static void draw(Buffer *b, size_t cx, size_t cy, Mode mode, const char *status) {
    system("cls");
    for (size_t i = 0; i < b->count; ++i) {
        if (i == cy) {
            // show cursor line with marker
            printf("> %s\n", b->lines[i]);
        } else {
            printf("  %s\n", b->lines[i]);
        }
    }
    printf("\n-- MODE: %s -- %s\n", mode == MODE_INSERT ? "INSERT" : (mode==MODE_COMMAND?"COMMAND":"NORMAL"), status ? status : "");
}

int main(int argc, char **argv) {
    Buffer buf;
    buffer_init(&buf);
    const char *path = NULL;
    if (argc >= 2) path = argv[1];
    if (path) {
        if (buffer_load(&buf, path) != 0) {
            fprintf(stderr, "Could not open '%s' - starting empty buffer\n", path);
            buf.lines[0] = (char*)malloc(1); buf.lines[0][0] = '\0'; buf.count = 1;
        }
    } else {
        buf.lines[0] = (char*)malloc(1); buf.lines[0][0] = '\0'; buf.count = 1;
    }

    size_t cx = 0, cy = 0;
    Mode mode = MODE_NORMAL;
    char status[128] = "";

    draw(&buf, cx, cy, mode, status);

    while (1) {
        int c = _getch();
        if (mode == MODE_NORMAL) {
            if (c == ':') {
                // command mode - simple single char commands w and q
                mode = MODE_COMMAND;
                draw(&buf, cx, cy, mode, ":");
                int cmd = _getch();
                if (cmd == 'w') {
                    if (path) {
                        if (buffer_save(&buf, path) == 0) strcpy(status, "Saved"); else strcpy(status, "Save failed");
                    } else {
                        strcpy(status, "No filename");
                    }
                } else if (cmd == 'q') {
                    buffer_free(&buf);
                    return 0;
                }
                mode = MODE_NORMAL;
                draw(&buf, cx, cy, mode, status);
                continue;
            }
            if (c == 'i') {
                mode = MODE_INSERT;
                strcpy(status, "");
                draw(&buf, cx, cy, mode, status);
                continue;
            }
            if (c == 'h') { if (cx>0) cx--; }
            else if (c == 'l') { if (cx < strlen(buf.lines[cy])) cx++; }
            else if (c == 'k') { if (cy>0) { cy--; if (cx > strlen(buf.lines[cy])) cx = strlen(buf.lines[cy]); } }
            else if (c == 'j') { if (cy+1 < buf.count) { cy++; if (cx > strlen(buf.lines[cy])) cx = strlen(buf.lines[cy]); } }
            else if (c == 0 || c == 224) {
                // arrow keys - second byte
                int c2 = _getch();
                if (c2 == 75) { if (cx>0) cx--; } // left
                else if (c2 == 77) { if (cx < strlen(buf.lines[cy])) cx++; } // right
                else if (c2 == 72) { if (cy>0) { cy--; if (cx > strlen(buf.lines[cy])) cx = strlen(buf.lines[cy]); } } // up
                else if (c2 == 80) { if (cy+1 < buf.count) { cy++; if (cx > strlen(buf.lines[cy])) cx = strlen(buf.lines[cy]); } } // down
            }
            draw(&buf, cx, cy, mode, status);
        } else if (mode == MODE_INSERT) {
            if (c == 27) { // ESC
                mode = MODE_NORMAL;
                draw(&buf, cx, cy, mode, status);
                continue;
            }
            if (c == '\r') { // Enter
                // split line
                char *line = buf.lines[cy];
                size_t len = strlen(line);
                char *left = (char*)malloc(cx + 1);
                strncpy(left, line, cx); left[cx] = '\0';
                char *right = (char*)malloc(len - cx + 1);
                strcpy(right, line + cx);
                free(buf.lines[cy]);
                buf.lines[cy] = left;
                // insert new line after cy
                if (buf.count + 1 < MAX_LINES) {
                    for (size_t i = buf.count; i > cy + 1; --i) buf.lines[i] = buf.lines[i-1];
                    buf.lines[cy+1] = right;
                    buf.count++;
                    cy++; cx = 0;
                } else {
                    // cannot insert
                    free(right);
                }
            } else if (c == 8) { // Backspace
                if (cx > 0) {
                    char *line = buf.lines[cy];
                    size_t len = strlen(line);
                    for (size_t i = cx-1; i < len; ++i) line[i] = line[i+1];
                    cx--;
                } else if (cx == 0 && cy > 0) {
                    // join with previous line
                    size_t prevlen = strlen(buf.lines[cy-1]);
                    size_t curlen = strlen(buf.lines[cy]);
                    buf.lines[cy-1] = (char*)realloc(buf.lines[cy-1], prevlen + curlen + 1);
                    strcat(buf.lines[cy-1], buf.lines[cy]);
                    free(buf.lines[cy]);
                    for (size_t i = cy; i < buf.count-1; ++i) buf.lines[i] = buf.lines[i+1];
                    buf.count--; cy--; cx = prevlen;
                }
            } else if (c == 0 || c == 224) {
                int c2 = _getch();
                // ignore arrows in insert for now
            } else if (c >= 32 && c < 127) {
                // insert printable char
                char *line = buf.lines[cy];
                size_t len = strlen(line);
                if (cx > len) cx = len; // clamp cursor to line length
                if (len + 1 >= LINE_CAP) continue; // leave room for new char + NUL
                char *newline = (char*)realloc(line, len + 2); // len + 1 char + NUL
                if (!newline) continue; // allocation failed
                // shift characters right starting from the end
                for (size_t i = len; i > cx; --i) newline[i] = newline[i-1];
                newline[cx] = (char)c;
                newline[len+1] = '\0';
                buf.lines[cy] = newline;
                cx++;
            }
            draw(&buf, cx, cy, mode, status);
        }
    }

    buffer_free(&buf);
    return 0;
}
