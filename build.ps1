# PowerShell build script for Windows (uses gcc)
Param(
    [string]$Target = "vte",
    [switch]$Clean
)

if ($PSBoundParameters.ContainsKey('Clean')) {
    if (Test-Path -Path .\bin) { Remove-Item -Recurse -Force .\bin }
    Write-Host "Cleaned bin/"
    exit 0
}

if (-not (Get-Command gcc -ErrorAction SilentlyContinue)) {
    Write-Error "gcc not found on PATH. Install MSYS2/Mingw or WSL and add gcc to PATH."
    exit 1
}

New-Item -ItemType Directory -Force -Path .\bin | Out-Null

if ($Target -eq 'vte') {
    $src = @(Join-Path -Path $PSScriptRoot -ChildPath "src\editor_curses.c"), (Join-Path -Path $PSScriptRoot -ChildPath "src\config.c"), (Join-Path -Path $PSScriptRoot -ChildPath "src\modules\line_edit.c"), (Join-Path -Path $PSScriptRoot -ChildPath "src\modules\buffer.c"), (Join-Path -Path $PSScriptRoot -ChildPath "src\modules\syntax.c"), (Join-Path -Path $PSScriptRoot -ChildPath "src\modules\navigation.c"), (Join-Path -Path $PSScriptRoot -ChildPath "src\internal\resize.c")
    $exe = Join-Path -Path $PSScriptRoot -ChildPath "bin\vte.exe"
    $linkopts = "-lpdcurses"
}
else {
    Write-Error "Unknown target: $Target"
    exit 1
}

gcc -Wall -Wextra -O2 -o $exe $src $linkopts
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed"; exit $LASTEXITCODE }

Write-Host "Built: $exe"
& $exe
