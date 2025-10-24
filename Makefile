CC = gcc
CFLAGS = -Wall -Wextra -O2
CURSES_SRC = src/editor_curses.c
VTE = bin\vte.exe

all: vte

vte: $(CURSES_SRC)
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CFLAGS) -o "$@" "$(CURSES_SRC)" -lpdcurses

clean:
	@if exist bin rmdir /s /q bin

.PHONY: all clean
