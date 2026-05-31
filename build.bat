@echo off
chcp 65001 > nul
echo Сборка проекта "Minesweeper"...

:: Компиляция с локальным Raylib
gcc main.c -o minesweeper.exe -I./raylib/include -L./raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==========================================
    echo Создан файл minesweeper.exe
    echo ==========================================
) else (
    echo.
    echo Ошибка во время компиляции.
)
pause