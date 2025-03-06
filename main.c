#include "raylib.h"
#include "inttypes.h"
#include "stdio.h"
#include "math.h"
#include <math.h>

#define SCREEN_LENGTH 800
#define BOARD_LENGTH 3

#define CELLSIZE (int)(SCREEN_LENGTH / BOARD_LENGTH)
#define PADDING (int)(CELLSIZE / 4)
#define LINE_THICKNESS 5


#define X 1
#define O 2
#define DRAW 3
#define NONE 0 

typedef struct {
    int currentPlayer;
    uint8_t grid[BOARD_LENGTH * BOARD_LENGTH];
    int gridSpaces;
    int winner;
} TicTacToeState;

typedef struct {
    int x;
    int y;
    float evalulation;
} MoveInfo;

void DrawTTTGrid();
void DrawPieceAt(int piece, int x, int y);
void InitState(TicTacToeState*);
void DrawState(TicTacToeState*);
void HandleInput();
// returns 1 if was a valid move otherwise 0
int MakeMove(TicTacToeState*, int x, int y);
int FindWinner(TicTacToeState*);

MoveInfo GetBestMove(TicTacToeState* state); 
void HandleAITurn();

float Minimax(TicTacToeState*, int, int);

// 0 1 2
// 3 4 5
// 6 7 8
int winPatterns[][3] = {
    {0,1,2},
    {3,4,5},
    {6,7,8},
    {0,3,6},
    {1,4,7},
    {2,5,8},
    {0,4,8},
    {2,4,6}
};
TicTacToeState globalState;

int aiPlayer = X;
int main(void)
{
    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(SCREEN_LENGTH, SCREEN_LENGTH, "tic tac toe");
    SetTargetFPS(60);

    InitState(&globalState);

    while (!WindowShouldClose())    
    {
        HandleInput();
        BeginDrawing();
            HandleAITurn();
            ClearBackground(RAYWHITE);
            DrawTTTGrid();
            DrawState(&globalState);
            switch (globalState.winner) {
                case NONE: break;
                case X: {
                    DrawText("X won!", PADDING, PADDING, 160, GRAY);
                } break;
                case O: {
                    DrawText("O won!", PADDING, PADDING, 160, GRAY);
                } break;
                case DRAW: {
                    DrawText("Draw :(", PADDING, PADDING, 160, GRAY);
                } break;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void HandleAITurn() {
    TraceLog(LOG_DEBUG, "handling ai move...");

    MoveInfo bestMove = GetBestMove(&globalState);
    TraceLog(LOG_DEBUG, "BestMove: %d %d", bestMove.x, bestMove.y);
    if (globalState.currentPlayer == aiPlayer) { 
        MakeMove(&globalState, bestMove.x, bestMove.y);
    }
}

void HandleInput() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (globalState.winner) {
            aiPlayer = (X - O) * (aiPlayer == O) + O;
            InitState(&globalState);
        } else if (globalState.currentPlayer != aiPlayer) {
            int x = GetMouseX() / CELLSIZE;
            int y = GetMouseY() / CELLSIZE;
            MakeMove(&globalState, x, y);
        }
    }
}

void InitState(TicTacToeState* state) {
    state->currentPlayer = X;
    state->gridSpaces = 0;
    for (size_t i = 0; i < BOARD_LENGTH * BOARD_LENGTH; ++i)
        state->grid[i] = NONE;
    state->winner = NONE;
}

int MakeMove(TicTacToeState* state, int x, int y) {
    if (state->grid[y * BOARD_LENGTH + x] != NONE || state->winner != NONE) return 0;
    state->grid[y * BOARD_LENGTH + x] = state->currentPlayer; 
    state->currentPlayer = (X - O) * (state->currentPlayer == O) + O;
    ++state->gridSpaces;
    state->winner = FindWinner(state);
    return 1;
}

float max(float a, float b) {
    if (a == NAN) return b;
    if (b == NAN) return a;
    if (a > b) return a;
    return b;
}

float min(float a, float b) {
    if (a == NAN) return b;
    if (b == NAN) return a;
    if (a < b) return a;
    return b;
}

MoveInfo GetBestMove(TicTacToeState* state) {
    int maximisingPlayer = state->currentPlayer == X;
    
    MoveInfo bestMove = {.x=-1,.y=-1};
    for (int y = 0; y < BOARD_LENGTH; ++y)
        for (int x = 0; x < BOARD_LENGTH; ++x) {
            TicTacToeState snapshot = *state;
            if (MakeMove(state, x, y)) {
                float score = Minimax(state, !maximisingPlayer, 0); 
                
                if (bestMove.x == -1 || (maximisingPlayer && score > bestMove.evalulation) || (!maximisingPlayer && score < bestMove.evalulation)) {
                    bestMove.evalulation = score;
                    bestMove.x = x;
                    bestMove.y = y;
                }
            }
            *state = snapshot;
        }
    
    return bestMove;
}

float Evaluate(TicTacToeState* state) {
    float eval = 0;

    for (int x = 0; x < BOARD_LENGTH; ++x) {
        for (int y = 0; y <= BOARD_LENGTH - 3; ++y) {
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;

            int close = 0;
            for (int dy = 1; dy < 3; ++dy) {
                if (firstValue == state->grid[(dy + y) * BOARD_LENGTH + x]) {
                    ++close;
                }
            }

            if (firstValue == X)
                eval += close * 5;
            else
                eval -= close * 5;
        }
    }

    for (int y = 0; y < BOARD_LENGTH; ++y) {
        for (int x = 0; x <= BOARD_LENGTH - 3; ++x) {
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;

            int close = 0;
            for (int dx = 1; dx < 3; ++dx) {
                if (firstValue != state->grid[y * BOARD_LENGTH + x + dx]) {
                    ++close;
                }
            }
            if (firstValue == X)
                eval += close * 5;
            else
                eval -= close * 5;
        }
    }

    for (int y = 0; y <= BOARD_LENGTH - 3; ++y) {
        for (int x = 0; x <= BOARD_LENGTH - 3; ++x) {
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;

            int close = 0;
            for (int d = 1; d < 3; ++d) {
                if (firstValue != state->grid[(d + y) * BOARD_LENGTH + x + d]) {
                    ++close;
                }
            }
            if (firstValue == X)
                eval += close * 5;
            else
                eval -= close * 5;
        }
    }

    for (int y = 0; y <= BOARD_LENGTH - 3; ++y) {
        for (int x2 = 0; x2 <= BOARD_LENGTH - 3; ++x2) {
            int x = BOARD_LENGTH - x2 - 1;
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;

            int close = 0;
            if (firstValue == NONE) continue;
            for (int d = 1; d < 3; ++d) {
                if (firstValue != state->grid[(d + y) * BOARD_LENGTH + x - d]) {
                    ++close;
                }
            }
            if (firstValue == X)
                eval += close * 5;
            else
                eval -= close * 5;
        }
    }
    return eval;
}

float Minimax(TicTacToeState* state, int maximisingPlayer, int depth) {
    if (state->winner != NONE) {
        switch (state->winner) {
            case X: return 100 - depth;
            case O: return -100 + depth;
            case DRAW: return 0;
        }
    }
    
    TicTacToeState snapshot;

    float bestScore = NAN;
    for (int y = 0; y < BOARD_LENGTH; ++y)
        for (int x = 0; x < BOARD_LENGTH; ++x) {
            snapshot = *state;
            if (MakeMove(state, x, y)) {
                float score = Minimax(state, !maximisingPlayer, depth + 1);
                if (maximisingPlayer)
                    bestScore = max(bestScore, score);
                else
                    bestScore = min(bestScore, score);
            }
            *state = snapshot;
        }
    
    return bestScore;
}

int FindWinner(TicTacToeState* state) {
    for (int i = 0; i < 8; ++i) {
        int* p = winPatterns[i];
        if (state->grid[p[0]] == state->grid[p[1]] && state->grid[p[2]] == state->grid[p[0]])
            return state->grid[*p];
    }

    return DRAW * (state->gridSpaces == 9);
}
/*
int FindWinner(TicTacToeState* state) {
    for (int x = 0; x < BOARD_LENGTH; ++x) {
        for (int y = 0; y <= BOARD_LENGTH - 3; ++y) {
            int isThree = 1;
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;
            for (int dy = 1; dy < 3; ++dy) {
                if (firstValue != state->grid[(dy + y) * BOARD_LENGTH + x]) {
                    isThree = 0;
                    break;
                }
            }
            if (isThree) return firstValue;
        }
    }

    for (int y = 0; y < BOARD_LENGTH; ++y) {
        for (int x = 0; x <= BOARD_LENGTH - 3; ++x) {
            int isThree = 1;
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;
            for (int dx = 1; dx < 3; ++dx) {
                if (firstValue != state->grid[y * BOARD_LENGTH + x + dx]) {
                    isThree = 0;
                    break;
                }
            }
            if (isThree) return firstValue;
        }
    }

    for (int y = 0; y <= BOARD_LENGTH - 3; ++y) {
        for (int x = 0; x <= BOARD_LENGTH - 3; ++x) {
            int isThree = 1;
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;
            for (int d = 1; d < 3; ++d) {
                if (firstValue != state->grid[(d + y) * BOARD_LENGTH + x + d]) {
                    isThree = 0;
                    break;
                }
            }
            if (isThree) return firstValue;
        }
    }

    for (int y = 0; y <= BOARD_LENGTH - 3; ++y) {
        for (int x2 = 0; x2 <= BOARD_LENGTH - 3; ++x2) {
            int x = BOARD_LENGTH - x2 - 1; 

            int isThree = 1;
            int firstValue = state->grid[y * BOARD_LENGTH + x]; 
            if (firstValue == NONE) continue;
            for (int d = 1; d < 3; ++d) {
                if (firstValue != state->grid[(d + y) * BOARD_LENGTH + x - d]) {
                    isThree = 0;
                    break;
                }
            }
            if (isThree) return firstValue;
        }
    }

    return DRAW * (state->gridSpaces == BOARD_LENGTH * BOARD_LENGTH);
}*/

void DrawTTTGrid() {
    for (int n = CELLSIZE; n < SCREEN_LENGTH; n += CELLSIZE) { 
        DrawLineEx((Vector2){0, n}, (Vector2){SCREEN_LENGTH, n}, LINE_THICKNESS, BLACK);
        DrawLineEx((Vector2){n, 0}, (Vector2){n, SCREEN_LENGTH}, LINE_THICKNESS, BLACK);
    }
}

void DrawState(TicTacToeState* state) {
    int i = 0;
    for (int y = 0; y < BOARD_LENGTH; ++y)
        for (int x = 0; x < BOARD_LENGTH; ++x)
            DrawPieceAt(state->grid[i++], x, y);
}

void DrawPieceAt(int piece, int x, int y) {
    switch(piece){
        case NONE: break;
        case O: {
            int centreX = (x + 0.5) * CELLSIZE;
            int centreY = (y + 0.5) * CELLSIZE;
            float radius = (CELLSIZE - PADDING) / 2.0f;

            DrawCircle(centreX, centreY, radius, BLACK);
            DrawCircle(centreX, centreY, radius - LINE_THICKNESS, RAYWHITE);
        } break;
        case X: {
            int leftX = x * CELLSIZE + PADDING;
            int rightX = leftX + CELLSIZE - PADDING * 2;
            int upperY = y * CELLSIZE + PADDING;
            int lowerY = upperY + CELLSIZE - PADDING * 2;

            DrawLineEx((Vector2){leftX, upperY}, (Vector2){rightX, lowerY}, LINE_THICKNESS, BLACK);
            DrawLineEx((Vector2){rightX, upperY}, (Vector2){leftX, lowerY}, LINE_THICKNESS, BLACK);
        } break;
    }
}
