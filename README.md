# Minimal C project

This repository is a tiny C project scaffold for Windows. It includes a simple `Hello, C world!` program, a Makefile, and convenience build scripts for PowerShell and cmd.

## Contents

- `src/main.c` — simple C program
- `Makefile` — GNU Make rules (also usable from MSYS2/MinGW)
- `build.ps1` — PowerShell build script (native Windows)
- `build.bat` — Batch build script (cmd.exe)
- `.vscode/settings.json` — workspace settings
- `.gitignore` — recommended ignores

## Prerequisites

- A C compiler (GCC from MSYS2/MinGW, or GCC in WSL). If you don't have one, you can:
  - Install MSYS2 (https://www.msys2.org/) and then install the `mingw-w64` toolchain
  - Or enable WSL and install `build-essential` in your Linux distro

## Quick start

### PowerShell (recommended on native Windows)

```powershell
# Build and run vte (curses-based editor)
.\build.ps1

# Clean
.\build.ps1 -Clean
```

### Command Prompt (cmd.exe)

```bat
:: Build and run
build.bat

:: Clean
build.bat clean
```

To build/run the editor using `build.bat`:

```bat
:: Build and run vte
build.bat
```

### Using Make (MSYS2/MinGW or WSL)

```powershell
# If you have make available (e.g. via MSYS2)
mingw32-make   # or just 'make'

# Run
.\bin\vte.exe
```

### WSL (Ubuntu)

```bash
make
./bin/main.exe
```

## Troubleshooting

- "gcc not found": install MSYS2/MinGW or WSL and ensure `gcc` is on your PATH. For MSYS2, add `C:\msys64\mingw64\bin` to your user PATH after installation.
- Permission errors when running scripts: in PowerShell you may need to adjust execution policy (run PowerShell as admin and use `Set-ExecutionPolicy RemoteSigned -Scope CurrentUser`).

## Editor: vte

vte is a minimal, curses-based, vim-like editor for Windows (PDCurses) or WSL (ncurses). Features:

- Normal mode: navigation with h/j/k/l and arrow keys
- Insert mode: press `i` to enter, Escape to return to Normal
- Command mode: press `:` and enter commands like `:w`, `:w filename`, `:q`, `:wq`
- Open/save a single file passed as an argument

Usage:

```powershell
# Open a file (or create it if missing)
.\bin\vte.exe path\to\file.txt
```

If you want more features (undo, search, multi-file, syntax highlighting), say which ones to prioritize and I'll add them.
