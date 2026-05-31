#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_GRID_SIZE 30
#define TOP_PANEL_HEIGHT 120

int CELL_SIZE = 45;

typedef struct {
    bool isMine;
    bool isRevealed;
    bool isFlagged;
    int neighborMines;
} Cell;

typedef enum { SCREEN_MENU, SCREEN_GAME } GameScreen;

Cell board[MAX_GRID_SIZE][MAX_GRID_SIZE];
GameScreen currentScreen = SCREEN_MENU;

int currentGridSize = 10;
int currentMineCount = 15;
int currentLevel = 1;

bool gameOver = false;
bool gameWon = false;
bool minesGenerated = false;

Color COLOR_BG       = (Color){ 15, 15, 22, 255 };
Color COLOR_PANEL    = (Color){ 22, 22, 33, 255 };
Color COLOR_CELL_NEW = (Color){ 33, 33, 48, 255 };
Color COLOR_CELL_HOV = (Color){ 45, 45, 68, 255 };
Color COLOR_CELL_OPN = (Color){ 20, 20, 28, 255 };
Color COLOR_CYAN     = (Color){ 0, 242, 254, 255 };
Color COLOR_PURPLE   = (Color){ 184, 38, 255, 255 };
Color COLOR_FLAG     = (Color){ 255, 42, 109, 255 };
Color COLOR_BTN      = (Color){ 50, 50, 70, 255 };
Color COLOR_BTN_HOV  = (Color){ 70, 70, 90, 255 };

Color GetNumberColor(int num) {
    switch (num) {
        case 1: return (Color){ 0, 162, 255, 255 };
        case 2: return (Color){ 50, 215, 75, 255 };
        case 3: return (Color){ 255, 69, 58, 255 };
        case 4: return (Color){ 175, 82, 222, 255 };
        case 5: return (Color){ 255, 159, 10, 255 };
        default: return (Color){ 255, 214, 10, 255 };
    }
}

void InitGame(void) {
    gameOver = false;
    gameWon = false;
    minesGenerated = false;
    for (int x = 0; x < currentGridSize; x++) {
        for (int y = 0; y < currentGridSize; y++) {
            board[x][y] = (Cell){ false, false, false, 0 };
        }
    }
}

void CountNeighborMines(void) {
    for (int x = 0; x < currentGridSize; x++) {
        for (int y = 0; y < currentGridSize; y++) {
            if (board[x][y].isMine) continue;
            int count = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < currentGridSize && ny >= 0 && ny < currentGridSize) {
                        if (board[nx][ny].isMine) count++;
                    }
                }
            }
            board[x][y].neighborMines = count;
        }
    }
}

void GenerateMines(int startX, int startY) {
    int placed = 0;
    while (placed < currentMineCount) {
        int x = rand() % currentGridSize;
        int y = rand() % currentGridSize;
        if ((x == startX && y == startY) || board[x][y].isMine) continue;
        board[x][y].isMine = true;
        placed++;
    }
    CountNeighborMines();
    minesGenerated = true;
}

void RevealCell(int x, int y) {
    if (x < 0 || x >= currentGridSize || y < 0 || y >= currentGridSize) return;
    if (board[x][y].isRevealed || board[x][y].isFlagged) return;
    board[x][y].isRevealed = true;
    if (board[x][y].neighborMines == 0 && !board[x][y].isMine) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                RevealCell(x + dx, y + dy);
            }
        }
    }
}

void CheckVictory(void) {
    int unrevealedSafeCells = 0;
    for (int x = 0; x < currentGridSize; x++) {
        for (int y = 0; y < currentGridSize; y++) {
            if (!board[x][y].isMine && !board[x][y].isRevealed) {
                unrevealedSafeCells++;
            }
        }
    }
    if (unrevealedSafeCells == 0) {
        gameWon = true;
    }
}

void RevealAllMines(void) {
    for (int x = 0; x < currentGridSize; x++) {
        for (int y = 0; y < currentGridSize; y++) {
            if (board[x][y].isMine) board[x][y].isRevealed = true;
        }
    }
}

bool DrawButton(Rectangle rect, const char* text) {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, rect);
    
    DrawRectangleRec(rect, isHovered ? COLOR_BTN_HOV : COLOR_BTN);
    DrawRectangleLinesEx(rect, 2, isHovered ? COLOR_CYAN : GRAY);
    
    int textWidth = MeasureText(text, 20);
    DrawText(text, rect.x + (rect.width - textWidth) / 2, rect.y + (rect.height - 20) / 2, 20, WHITE);
    
    return (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
}

void AdjustWindowSize(void) {
    int screenH = GetMonitorHeight(GetCurrentMonitor());
    int screenW = GetMonitorWidth(GetCurrentMonitor());
    if (screenH < 600) screenH = 1080;
    if (screenW < 800) screenW = 1920;

    int maxH = screenH - TOP_PANEL_HEIGHT - 100;
    CELL_SIZE = maxH / currentGridSize;
    if (CELL_SIZE > 45) CELL_SIZE = 45;
    if (CELL_SIZE < 15) CELL_SIZE = 15;

    int gw = currentGridSize * CELL_SIZE + 40;
    if (gw < 400) gw = 400; 
    int gh = currentGridSize * CELL_SIZE + TOP_PANEL_HEIGHT + 20;

    SetWindowSize(gw, gh);
    SetWindowPosition((screenW - gw) / 2, (screenH - gh) / 2);
}

int main(void) {
    srand(time(NULL));
    
    int menuWidth = 400;
    int menuHeight = 500;
    InitWindow(menuWidth, menuHeight, "Minesweeper - Menu");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        BeginDrawing();
        ClearBackground(COLOR_BG);

        if (currentScreen == SCREEN_MENU) {
            DrawText("SELECT LEVEL", menuWidth / 2 - MeasureText("SELECT LEVEL", 30) / 2, 80, 30, COLOR_CYAN);
            
            Rectangle btnEasy   = { menuWidth / 2 - 100, 180, 200, 50 };
            Rectangle btnMedium = { menuWidth / 2 - 100, 250, 200, 50 };
            Rectangle btnHard   = { menuWidth / 2 - 100, 320, 200, 50 };

            if (DrawButton(btnEasy, "EASY (10x10)")) {
                currentGridSize = 10;
                currentMineCount = 15;
                currentLevel = 1;
                currentScreen = SCREEN_GAME;
            }
            if (DrawButton(btnMedium, "MEDIUM (15x15)")) {
                currentGridSize = 15;
                currentMineCount = 40;
                currentLevel = 5;
                currentScreen = SCREEN_GAME;
            }
            if (DrawButton(btnHard, "HARD (20x20)")) {
                currentGridSize = 20;
                currentMineCount = 80;
                currentLevel = 10;
                currentScreen = SCREEN_GAME;
            }

            if (currentScreen == SCREEN_GAME) {
                AdjustWindowSize();
                SetWindowTitle("Minesweeper - Playing");
                InitGame();
            }

        } else if (currentScreen == SCREEN_GAME) {
            int boardStartX = (GetScreenWidth() - currentGridSize * CELL_SIZE) / 2;
            int boardStartY = TOP_PANEL_HEIGHT;

            if (IsKeyPressed(KEY_R)) InitGame();
            if (IsKeyPressed(KEY_M)) {
                currentScreen = SCREEN_MENU;
                SetWindowSize(menuWidth, menuHeight);
                SetWindowTitle("Minesweeper - Menu");
                int screenW = GetMonitorWidth(GetCurrentMonitor());
                int screenH = GetMonitorHeight(GetCurrentMonitor());
                SetWindowPosition((screenW - menuWidth) / 2, (screenH - menuHeight) / 2);
            }

            if (!gameOver && !gameWon) {
                if (mousePos.x >= boardStartX && mousePos.x < boardStartX + currentGridSize * CELL_SIZE &&
                    mousePos.y >= boardStartY && mousePos.y < boardStartY + currentGridSize * CELL_SIZE) {
                    
                    int cellX = (mousePos.x - boardStartX) / CELL_SIZE;
                    int cellY = (mousePos.y - boardStartY) / CELL_SIZE;

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (!minesGenerated) {
                            GenerateMines(cellX, cellY);
                        }
                        if (!board[cellX][cellY].isFlagged) {
                            if (board[cellX][cellY].isMine) {
                                gameOver = true;
                                RevealAllMines();
                            } else {
                                RevealCell(cellX, cellY);
                                CheckVictory();
                            }
                        }
                    }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                        if (!board[cellX][cellY].isRevealed) {
                            board[cellX][cellY].isFlagged = !board[cellX][cellY].isFlagged;
                        }
                    }
                }
            }

            DrawRectangle(0, 0, GetScreenWidth(), TOP_PANEL_HEIGHT - 20, COLOR_PANEL);
            
            DrawText(TextFormat("Level: %d", currentLevel), 25, 25, 28, COLOR_CYAN);
            DrawText(TextFormat("Mines: %d", currentMineCount), GetScreenWidth() - MeasureText(TextFormat("Mines: %d", currentMineCount), 28) - 25, 25, 28, COLOR_FLAG);
            DrawText("L-Click: Open | R-Click: Flag | R: Restart | M: Menu", GetScreenWidth() / 2 - MeasureText("L-Click: Open | R-Click: Flag | R: Restart | M: Menu", 13) / 2, 65, 13, GRAY);

            for (int x = 0; x < currentGridSize; x++) {
                for (int y = 0; y < currentGridSize; y++) {
                    int posX = boardStartX + x * CELL_SIZE;
                    int posY = boardStartY + y * CELL_SIZE;
                    bool isHovered = (mousePos.x >= posX && mousePos.x < posX + CELL_SIZE &&
                                      mousePos.y >= posY && mousePos.y < posY + CELL_SIZE);

                    if (board[x][y].isRevealed) {
                        DrawRectangle(posX + 2, posY + 2, CELL_SIZE - 4, CELL_SIZE - 4, COLOR_CELL_OPN);
                        if (board[x][y].isMine) {
                            DrawCircle(posX + CELL_SIZE / 2, posY + CELL_SIZE / 2, CELL_SIZE / 4, COLOR_FLAG);
                        } else if (board[x][y].neighborMines > 0) {
                            DrawText(TextFormat("%d", board[x][y].neighborMines), 
                                     posX + CELL_SIZE / 2 - 6, posY + CELL_SIZE / 2 - 10, 
                                     20, GetNumberColor(board[x][y].neighborMines));
                        }
                    } else {
                        Color currentCellColor = (isHovered && !gameOver && !gameWon) ? COLOR_CELL_HOV : COLOR_CELL_NEW;
                        DrawRectangle(posX + 2, posY + 2, CELL_SIZE - 4, CELL_SIZE - 4, currentCellColor);
                        if (board[x][y].isFlagged) {
                            DrawRectangle(posX + (int)(CELL_SIZE*0.4), posY + (int)(CELL_SIZE*0.3), 2, (int)(CELL_SIZE*0.4), COLOR_FLAG);
                            DrawTriangle(
                                (Vector2){ posX + (int)(CELL_SIZE*0.4) + 2, posY + (int)(CELL_SIZE*0.3) },
                                (Vector2){ posX + (int)(CELL_SIZE*0.4) + 2, posY + (int)(CELL_SIZE*0.5) },
                                (Vector2){ posX + (int)(CELL_SIZE*0.7), posY + (int)(CELL_SIZE*0.4) },
                                COLOR_FLAG
                            );
                        }
                    }
                    DrawRectangleLines(posX, posY, CELL_SIZE, CELL_SIZE, (Color){ 40, 40, 60, 100 });
                }
            }

            if (gameOver || gameWon) {
                DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 10, 10, 15, 200 });
                if (gameOver) {
                    DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 36) / 2, GetScreenHeight() / 2 - 40, 36, COLOR_FLAG);
                    DrawText("Press 'R' to Try Again or 'M' for Menu", GetScreenWidth() / 2 - MeasureText("Press 'R' to Try Again or 'M' for Menu", 16) / 2, GetScreenHeight() / 2 + 15, 16, LIGHTGRAY);
                } else {
                    DrawText("VICTORY!", GetScreenWidth() / 2 - MeasureText("VICTORY!", 36) / 2, GetScreenHeight() / 2 - 60, 36, COLOR_CYAN);
                    
                    Rectangle btnNext = { GetScreenWidth() / 2 - 100, GetScreenHeight() / 2 - 10, 200, 50 };
                    if (DrawButton(btnNext, "NEXT LEVEL")) {
                        currentLevel++;
                        
                        if (currentGridSize < MAX_GRID_SIZE) {
                            currentGridSize++;
                        }
                        currentMineCount += 5;
                        
                        int maxMines = (currentGridSize * currentGridSize) - 9;
                        if (currentMineCount > maxMines) {
                            currentMineCount = maxMines;
                        }
                        
                        AdjustWindowSize();
                        InitGame();
                    }
                    
                    DrawText("Press 'M' for Menu", GetScreenWidth() / 2 - MeasureText("Press 'M' for Menu", 14) / 2, GetScreenHeight() / 2 + 60, 14, LIGHTGRAY);
                }
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}