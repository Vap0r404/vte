# VTE

`vte` is a tiny cross-platform text editor written **COMPLETELY by AI** in C that uses curses (PDCurses on Windows, ncurses on Unix/Linux/macOS).
It provides a minimal, vim-inspired editing experience: Normal/Insert/Command modes, simple navigation, basic line editing, and multi-buffer support.

## What this project is

- A minimal, educational curses-based text editor
- Cross-platform: Works on Windows, Linux, macOS, and WSL
- Focus is on small, easy-to-read C code with curses UI, simple line-editor logic, and a tiny command mode
- Intended as a small and simple replacement for CLI text editors and a starting point for experimentation — add features if you want

## Features

- **Vim-like modes**: Normal, Insert, Command, Search
- **Multi-buffer support**: Up to 16 files open at once (`:bn`, `:bp` to switch)
- **UTF-8 support**: Display and editing with codepoint-aware operations
- **Visual line wrapping**: Column-aware rendering with wrap cache
- **Mouse support**: Click to position cursor in both Normal and Insert modes
- **Undo/Redo**: Full undo and redo support with Ctrl+Z and Ctrl+Y
- **Clipboard**: Yank (copy) and paste lines with `y` and `p`
- **Configuration**: `.vterc` file with `:set` commands
- **Search**: Forward/backward pattern search with wrapping (`/`, `n`, `N`)
- **Modular architecture**: Clean separation of concerns (modules, internal utilities, platform layer)

## Project structure

- `src/editor_curses.c` — The core editor loop
- `src/config.c` / `src/config.h` — Configuration system (`:set` commands, `.vterc` file)
- `src/modules/` — Modular components (line editing, buffers, syntax, navigation, status)
- `src/internal/` — Internal utilities (resize, mouse, UTF-8, wrapping, cache)
- `src/platform/` — Platform abstraction layer (Windows/\*Unix compatibility)
- `build.ps1` — PowerShell build script (Windows)
- `build.bat` — Batch build script (Windows cmd.exe)
- `build.sh` — Bash build script (Unix/Linux/macOS)
- `Makefile` — Cross-platform GNU Make rules

## Prerequisites

### Windows

- **Compiler**: GCC from [MSYS2/MinGW](https://www.msys2.org/) or WSL
- **Curses library**: PDCurses (narrow) or pdcursesw (wide, recommended for better Unicode support)

### Linux/Unix/macOS

- **Compiler**: GCC or Clang (usually pre-installed or via `build-essential`/`base-devel`)
- **Curses library**: ncurses (usually pre-installed)
  - Debian/Ubuntu: `sudo apt install libncurses-dev`
  - Arch: `sudo pacman -S ncurses`
  - macOS: pre-installed or via Homebrew

## Quick start

### Windows (PowerShell)

```powershell
# Build and run
.\build.ps1

# Build with wide-character support **(if pdcursesw installed)**
.\build.ps1 -Wide

# Clean
.\build.ps1 -Clean
```

### Windows (cmd)

```bat
:: Build and run
build.bat

:: Clean
build.bat clean
```

### Linux/Unix/macOS (bash)

```bash
# Build
chmod +x build.sh
./build.sh

# Run
./bin/vte [filename]

# Clean
./build.sh clean
```

### Cross-platform (make)

```bash
# Build (auto-detects Windows vs Unix)
make

# Clean
make clean
```

## Usage

```bash
# Windows
.\bin\vte.exe [filename]

# Unix/Linux/macOS
./bin/vte [filename]
```

- **Normal mode**: Navigate with `h/j/k/l` or arrow keys. Press `i` to enter INSERT mode.
  - `y` — yank (copy) current line to clipboard
  - `p` — paste clipboard content (line or character)
  - `Ctrl+Z` — undo last change
  - `Ctrl+Y` — redo last undone change
- **Insert mode**: Type to insert text, Backspace removes characters, Enter splits the line. Press `Esc` to return to Normal.
- **Command mode**: Press `:` then type commands:
  - `:w` — save current buffer
  - `:w filename` — save as
  - `:e filename` — open file in new buffer
  - `:bn` / `:bp` — next/previous buffer
  - `:q` — quit (all buffers)
  - `:wq` — save and quit
  - `:123` — goto line 123
  - `:h` or `:help` — show help
  - `:set` — show settings
- **Search mode**: Press `/` then type pattern, `n` for next match, `N` for previous

## Platform notes

### Windows

- **UTF-8 input**: Some dead-key combinations may not work with narrow pdcurses. For best results:
  - Use Windows Terminal with UTF-8 locale
  - Build with `-Wide` flag (requires pdcursesw)
  - Or run in WSL where ncurses handles Unicode natively
- **Binary releases** include precompiled `vte.exe` built with narrow pdcurses

### Linux/Unix/macOS

- UTF-8 works out of the box with ncurses
- Make sure your terminal locale is set to UTF-8 (`LANG=en_US.UTF-8` or similar)
- Mouse support requires xterm-compatible terminal

## Architecture

vte uses a clean modular design:

- **Platform layer** (`src/platform/`): Abstracts Windows vs Unix differences (console setup, code pages)
- **Core editor** (`src/editor_curses.c`): Main loop, mode handling, rendering
- **Modules** (`src/modules/`): Self-contained components (buffers, line editing, navigation, syntax, status)
- **Internal utilities** (`src/internal/`): UTF-8 handling, wrapping, mouse, resize, caching

This makes it easy to add features, port to new platforms, or understand the codebase.

## Development

- **Adding features**: Most features go in `src/modules/` or `src/internal/`
- **Platform-specific code**: Goes in `src/platform/platform.c` with `#ifdef` guards
- **Build system**: Update all three build scripts (`.ps1`, `.bat`, `.sh`) and `Makefile` when adding files
- **Testing**: Test on both Windows and Unix when making changes to core or platform code---

If you maintain a fork or add features, consider updating this README with a short summary of the added capabilities.
