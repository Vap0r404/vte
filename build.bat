@echo off
REM Simple build script for vte (curses-based editor)
IF "%1"=="clean" (
    if exist bin rmdir /s /q bin
    echo Cleaned bin\
    goto :eof
)

where gcc >nul 2>&1 || (
    echo gcc not found on PATH. Install MSYS2/Mingw or WSL and add gcc to PATH.
    exit /b 1
)

if not exist bin mkdir bin

set TARGET=%1
if "%TARGET%"=="" set TARGET=vte

if /I "%TARGET%"=="vte" (
    gcc -Wall -Wextra -O2 -o "bin\\vte.exe" "src\\editor_curses.c" "src\\config.c" "src\\modules\\line_edit.c" "src\\modules\\buffer.c" "src\\modules\\syntax.c" "src\\modules\\navigation.c" "src\\modules\\status.c" "src\\internal\\resize.c" "src\\internal\\mouse.c" "src\\internal\\wrap.c" "src\\internal\\wrap_cache.c" "src\\internal\\utf8.c" "src\\internal\\utf8_edit.c" "src\\platform\\platform.c" -lpdcurses
    if errorlevel 1 (
        echo Build failed
        exit /b 1
    )
    echo Built bin\vte.exe
    bin\vte.exe
    goto :eof
)

echo Unknown target %TARGET%
exit /b 1
