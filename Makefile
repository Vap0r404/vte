CC = gcc
CFLAGS = -Wall -Wextra -O2
ifdef WIDE
	CFLAGS += -DPDC_WIDE
	LIBCURSES = -lpdcursesw
else
	LIBCURSES = -lpdcurses
endif
CURSES_SRC = src/editor_curses.c src/config.c src/modules/line_edit.c src/modules/buffer.c src/modules/syntax.c src/modules/navigation.c src/modules/status.c src/internal/resize.c src/internal/mouse.c src/internal/wrap.c src/internal/wrap_cache.c src/internal/utf8.c src/internal/utf8_edit.c
VTE = bin\vte.exe

all: vte

vte: $(CURSES_SRC)
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	$(CC) $(CFLAGS) -o "$@" $(CURSES_SRC) $(LIBCURSES)

clean:
	@if exist bin rmdir /s /q bin

.PHONY: all clean
