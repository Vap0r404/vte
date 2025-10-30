#!/bin/bash
# Unix/Linux/macOS build script for vte

set -e

# Clean build
if [ "$1" = "clean" ]; then
    rm -rf bin
    echo "Cleaned bin/"
    exit 0
fi

# Check for gcc
if ! command -v gcc &> /dev/null; then
    echo "gcc not found. Please install build-essential (Debian/Ubuntu) or base-devel (Arch)."
    exit 1
fi

# Create bin directory
mkdir -p bin

# Build with ncurses
echo "Building vte for Unix/Linux..."
gcc -Wall -Wextra -O2 -D_XOPEN_SOURCE_EXTENDED \
    -o bin/vte \
    src/editor_curses.c \
    src/config.c \
    src/modules/line_edit.c \
    src/modules/buffer.c \
    src/modules/syntax.c \
    src/modules/navigation.c \
    src/modules/status.c \
    src/internal/resize.c \
    src/internal/mouse.c \
    src/internal/wrap.c \
    src/internal/wrap_cache.c \
    src/internal/utf8.c \
    src/internal/utf8_edit.c \
    src/platform/platform.c \
    -lncurses

if [ $? -eq 0 ]; then
    echo "Built: bin/vte"
    echo ""
    echo "Run with: ./bin/vte [filename]"
else
    echo "Build failed"
    exit 1
fi
