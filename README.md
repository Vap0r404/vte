# VTE

`vte` is a tiny, single-file text editor written **COMPLETELY by AI** in C that uses a curses implementation (PDCurses on Windows or ncurses on Unix/WSL).
It provides a minimal, vim-inspired editing experience: Normal/Insert/Command modes, simple navigation, basic line editing, and single-file open/save.

## What this project is

- A minimal, educational curses-based text editor.
- Focus is on small, easy-to-read C code with curses UI, simple line-editor logic, and a tiny command mode.
- Intended as a small and simple replacement for CLI text editors and a starting point for experimentation — add features if you want.

## Contents

- `src/editor_curses.c` — The core editor
- `src/config.c` / `src/config.h` — Configuration system (:set commands, .vterc file)
- `src/modules/` — Modular components (line editing, buffers, syntax, navigation)
- `src/internal/` — Internal utilities (resize, mouse etc.)
- `Makefile` — GNU Make rules (also usable from MSYS2/MinGW)
- `build.ps1` — PowerShell build script (native Windows)
- `build.bat` — Batch build script (cmd.exe)

## Prerequisites

- A C compiler (GCC from MSYS2/MinGW, or GCC in WSL). If you don't have one, you can:
  - Install MSYS2 (https://www.msys2.org/) and then install the `mingw-w64` toolchain
  - Or enable WSL and install `build-essential` in your Linux distro
  - PDCurses and the library

## This repository contains the editor source and convenient build scripts for Windows and MSYS2/MinGW/WSL.

# Quick start

## Prerequisites

- A C compiler (GCC is the usual choice). On Windows you can use MSYS2/MinGW or WSL. On Windows the project links with PDCurses; on WSL/nix use ncurses.

## Build & run (Windows, PowerShell)

```powershell
# Build and run (PowerShell)
.\build.ps1

# Clean
.\build.ps1 -Clean
```

## Build & run (cmd)

```bat
:: Build and run
build.bat

:: Clean
build.bat clean
```

## Build & run (MSYS2 / WSL / make)

```bash
# If you have make available
mingw32-make   # or just 'make'

# Run the built executable
./bin/vte.exe
```

## Usage

- Start the editor with an optional filename: `./bin/vte.exe path\\to\\file.txt`
- Normal mode: navigate with `h/j/k/l` or arrow keys. Press `i` to enter INSERT mode.
- Insert mode: type to insert text, Backspace removes characters, Enter splits the line. Press `Esc` to return to Normal.
- Command mode: press `:` then type commands like `w`, `w filename`, `q`, `h`/`help` for more info.

## Notes

- This is intentionally small and synchronous — curses `getch()` is used in the main loop and the screen is redrawn each iteration.
- Line editing is implemented in `src/modules/line_edit.c`; the editor copies the in-progress buffer when leaving INSERT mode so you can actually see what you're typing in real time.
- If you want to help the project, feel free to do it.

## Unicode input on Windows

- Rendering and files are UTF‑8. Keyboard input in the Windows console depends on your terminal and the curses library variant available.
- On some setups (legacy `pdcurses` without wide‑char support), certain composed characters that use dead keys/AltGr (e.g. Polish ó, ą, ć, ń) may not be delivered to the app as a single Unicode codepoint. In those cases, insertion can fail or behave inconsistently.
- Workarounds:
  - Use a terminal that handles UTF‑8 and keyboard composition well (Windows Terminal, UTF‑8 locale/code page, a Unicode font like Consolas).
  - Prefer WSL/Linux where `ncursesw` wide input is standard.
  - Build with wide curses if available: `.
build.ps1 -Wide` (requires `pdcursesw` installed and linkable as `-lpdcursesw`). This enables `get_wch()` wide‑character input.
  - As a last resort, paste text or use OS input methods that produce precomposed characters directly.

If some Unicode characters still don't work in your console environment, it's a known limitation of the minimal curses input path on Windows. PRs to improve the Windows input layer (e.g. using `ReadConsoleInputW`) are very welcome.

---

If you maintain a fork or add features, consider updating this README with a short summary of the added capabilities.
