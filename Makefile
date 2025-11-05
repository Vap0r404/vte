CC = gcc
CFLAGS = -Wall -Wextra -O2

# Detect platform
ifeq ($(OS),Windows_NT)
    PLATFORM = Windows
    EXE_EXT = .exe
    MKDIR = if not exist "$(dir $@)" mkdir "$(dir $@)"
    RM = if exist bin rmdir /s /q bin
    ifdef WIDE
        CFLAGS += -DPDC_WIDE
        LIBCURSES = -lpdcursesw
    else
        LIBCURSES = -lpdcurses
    endif
else
    PLATFORM = Unix
    EXE_EXT =
    MKDIR = mkdir -p bin
    RM = rm -rf bin
    # On Unix, use ncurses (wide by default on modern systems)
    LIBCURSES = -lncurses
    # Enable wide character support
    CFLAGS += -D_XOPEN_SOURCE_EXTENDED
endif

CURSES_SRC = src/editor_curses.c src/config.c src/modules/line_edit.c src/modules/buffer.c src/modules/syntax.c src/modules/navigation.c src/modules/status.c src/modules/undo.c src/modules/clipboard.c src/internal/resize.c src/internal/mouse.c src/internal/wrap.c src/internal/wrap_cache.c src/internal/utf8.c src/internal/utf8_edit.c src/platform/platform.c
VTE = bin/vte$(EXE_EXT)

all: vte

vte: $(CURSES_SRC)
	@$(MKDIR)
	$(CC) $(CFLAGS) -o "$(VTE)" $(CURSES_SRC) $(LIBCURSES)

clean:
	@$(RM)

.PHONY: all clean
