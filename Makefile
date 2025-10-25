CC = gcc
CFLAGS = -Wall -Wextra -O2
CURSES_SRC = src/editor_curses.c src/config.c src/modules/line_edit.c src/modules/buffer.c src/modules/syntax.c src/modules/navigation.c
VTE = bin\vte.exe

all: vte

vte: $(CURSES_SRC)
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CFLAGS) -o "$@" $(CURSES_SRC) -lpdcurses

clean:
	@if exist bin rmdir /s /q bin

.PHONY: all clean
