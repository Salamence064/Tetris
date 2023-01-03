/*****************************************************************
*                                                                *
*   Copyright (c) 2022 Thomas Ducote (thomas.ducote64@gmail.com) *
*                                                                *
*****************************************************************/

#include <sstream>
#include "piece.h"
#include "levels.h"

// used to switch menus
// 0 == main menu, 1 == controls, 2 == game screen, 2 == game over screen
int gameState = 0;
const int menuState = 0;
const int controlsState = 1;
const int tetrisState = 2;
const int gameOverState = 3;

// local variables -- in terms of pixels
const int screenWidth = 1200;
const int screenHeight = 800;
const int tileSize = 32; 
const float fontScale = screenWidth/80;

// in terms of tile number
const int boardWidth = 10;
const int boardHeight = 20;
const int maxBuffer = 2; // numTiles a piece can be rotated up or down

// offset from the sides of the screen -- in terms of pixels
const int offsetX = screenWidth/2 - (boardWidth/2 * tileSize);
const int offsetY = screenHeight/2 - (boardHeight/2 * tileSize);

// delta time variable
float dt = 0.0f;
float grav = 0.0f;
bool canHold = 1;

// store the active piece based on indices -- there can only be one at a time
int activePiece[piece::numPoints][2];
int pieceType; // used for held pieces

// do we need to get a random starting piece?
bool startingPiece = 1;

// board
Color board[boardHeight][boardWidth] = {
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK},
    {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK}
};

// external functions
void level::drawScore() {
    std::ostringstream sout;
    sout << "Score: " << score;

    DrawText(sout.str().c_str(), fontScale, 4.5 * fontScale, 3.5 * fontScale, WHITE);
};

void level::drawLinesCleared() {
    std::ostringstream sout;
    sout << "Lines Cleared: " << linesCleared;

    DrawText(sout.str().c_str(), fontScale, fontScale, 3.5 * fontScale, WHITE);
};

void level::drawLevel() {
    std::ostringstream sout;
    sout << "Level: " << level;
    int textWidth = MeasureText(sout.str().c_str(), 3.5 * fontScale);

    DrawText(sout.str().c_str(), screenWidth - textWidth - fontScale, fontScale, 3.5 * fontScale, WHITE);
};

void level::displayScore() {
    std::ostringstream sout;
    sout << "Your Score: " << score;
    int textWidth = MeasureText(sout.str().c_str(), 5 * fontScale);
    
    DrawText(sout.str().c_str(), screenWidth/2 - textWidth/2, screenHeight/2 + 2.5 * fontScale, 5 * fontScale, WHITE);
};

void piece::drawHeldPiece() {
    int fontSize = 2.5 * fontScale;
    int x = offsetX + tileSize * boardWidth + fontScale;
    int y = offsetY + 2*fontScale + fontSize + 4*tileSize;

    DrawText("Held Piece", x, y, fontSize, WHITE);

    DrawLine(x, y + fontSize, x + 4*tileSize, y + fontSize, WHITE);
    DrawLine(x, y + fontSize, x, y + fontSize + 4*tileSize, WHITE);
    DrawLine(x + 4*tileSize, y + fontSize, x + 4*tileSize, y + fontSize + 4*tileSize, WHITE);
    DrawLine(x, y + fontSize + 4*tileSize, x + 4*tileSize, y + fontSize + 4*tileSize, WHITE);

    if (heldPiece == -1) { return; } // ensure there is a held piece

    Color color = colors[heldPiece];

    // draw the piece
    for (int i = 0; i < numPoints; i++) {
        int pieceY = indices[heldPiece][i][0] + 1, pieceX = indices[heldPiece][i][1] - 3;
        DrawRectangle(x + pieceX*tileSize, y + fontSize + pieceY*tileSize, tileSize, tileSize, color);
    }
};

void piece::drawNextPiece() {
    // randomize the order if the current index has been reset
    if (update) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(pieceOrder.begin(), pieceOrder.end(), g);
        update = 0;
    }

    int x = offsetX + tileSize * boardWidth + fontScale;
    int y = offsetY + fontScale;
    int fontSize = 2.5*fontScale;

    DrawText("Next Piece", x, y, fontSize, WHITE);

    // give the illusion of a box display through lines
    DrawLine(x, y + fontSize, x + 4*tileSize, y + fontSize, WHITE);
    DrawLine(x, y + fontSize, x, y + fontSize + 4*tileSize, WHITE);
    DrawLine(x + 4*tileSize, y + fontSize, x + 4*tileSize, y + fontSize + 4*tileSize, WHITE);
    DrawLine(x, y + fontSize + 4*tileSize, x + 4*tileSize, y + fontSize + 4*tileSize, WHITE);

    int type = pieceOrder[currIndex];
    Color color = colors[type];

    // draw the piece
    for (int i = 0; i < numPoints; i++) {
        int pieceY = indices[type][i][0] + 1, pieceX = indices[type][i][1] - 3;
        DrawRectangle(x + pieceX*tileSize, y + fontSize + pieceY*tileSize, tileSize, tileSize, color);
    }
};

// local functions
static void UpdateDrawFrame(void); // update and draw every frame
static void resetBoard(); // used to reset the board after a game over
static void drawBoard(); // draw the current state of the board
static void updateBoard(); // update the board based on gravity, rotations, and line clears -- returns 1 if the program should quit
static void movePiece(const int dir); // move the piece left or right
static void rotatePiece(const int dir); // used to rotate the piece left or right.
static void forceInBounds(int loc[4][2], const int dir); // force the piece to remain in bounds when rotating
static void clearLines(); // used to check if a line should be cleared
static void hardDropPiece(); // hard drop a piece -- returns 1 if program should quit
static void holdPiece(); // hold the current active piece

static bool hasLost(); // determines when the player loses
static bool hasPiece(const int row, const int col); // determine if the board square has a piece through checking rgbs
static bool isActive(const int row, const int col); // determine if a current square contains an active piece
static bool updatePiece(int cords[4][2], const int row, const int col, const bool vert, const int dir, int& mod); // update a rotating piece
static bool canMove(int cords[4][2], const int rowMod, const int colMod); // ensure the piece being rotated can move a certain amount


int main() {
    // initialization
    InitWindow(screenWidth, screenHeight, "Tetris");

    // Main game loop
    // Detect window close button or ESC key
    while (!WindowShouldClose()) { UpdateDrawFrame(); }

    // Close window and OpenGL context
    CloseWindow();
    return 0;
}

// Update and draw the game
static void UpdateDrawFrame(void) {
    // main menu screen
    if (gameState == menuState) {
        if (startingPiece) {
            // starting piece
            pieceType = piece::getPiece();
            Color color = piece::getColor();
            int** points = piece::getPoints();

            for (int i = 0; i < piece::numPoints; i++) {
                board[points[i][0]][points[i][1]] = color;
                activePiece[i][0] = points[i][0];
                activePiece[i][1] = points[i][1];
                delete[] points[i];
            }

            delete[] points;
            points = 0;

            startingPiece = 0;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { gameState = controlsState; }

        BeginDrawing();

            ClearBackground(BLACK);
            int textWidth = MeasureText("Tetris", 10 * fontScale);
            DrawText("Tetris", screenWidth/2 - textWidth/2, screenHeight/2 - 5 * fontScale, 10 * fontScale, WHITE);

        EndDrawing();

    } else if (gameState == controlsState) { // controls screen
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { gameState = tetrisState; }

        BeginDrawing();

            ClearBackground(BLACK);
            int textWidth = MeasureText("Controls", 5 * fontScale);
            int textWidth1 = MeasureText("|<-||->| - Move left and right", 2.5 * fontScale);
            int textWidth2 = MeasureText("|UP| - Rotate right", 2.5 * fontScale);
            int textWidth3 = MeasureText("|Z| - Rotate left", 2.5 * fontScale);
            int textWidth4 = MeasureText("|DOWN| - Soft drop", 2.5 * fontScale);
            int textWidth5 = MeasureText("|SPACE| - Hard drop", 2.5 * fontScale);
            int textWidth6 = MeasureText("|C| - Hold piece", 2.5 * fontScale);

            DrawText("Controls", screenWidth/2 - textWidth/2, screenHeight/2 - 15*fontScale, 5 * fontScale, WHITE);
            DrawText("|<-||->| - Move left and right", screenWidth/2 - textWidth1/2, screenHeight/2 - 3*2.5*fontScale, 2.5 * fontScale, WHITE);
            DrawText("|UP| - Rotate right", screenWidth/2 - textWidth2/2, screenHeight/2 - 2*2.5*fontScale, 2.5 * fontScale, WHITE);
            DrawText("|Z| - Rotate left", screenWidth/2 - textWidth3/2, screenHeight/2 - 2.5*fontScale, 2.5 * fontScale, WHITE);
            DrawText("|DOWN| - Soft drop", screenWidth/2 - textWidth4/2, screenHeight/2, 2.5 * fontScale, WHITE);
            DrawText("|SPACE| - Hard drop", screenWidth/2 - textWidth5/2, screenHeight/2 + 2.5*fontScale, 2.5 * fontScale, WHITE);
            DrawText("|C| - Hold piece", screenWidth/2 - textWidth6/2, screenHeight/2 + 2*2.5*fontScale, 2.5 * fontScale, WHITE);

        EndDrawing();

    } else if (gameState == tetrisState) { // game screen
        // delta time variable
        grav += GetFrameTime();
        dt += GetFrameTime();

        // Update
        // rotate pieces
        if (IsKeyPressed(KEY_UP)) { rotatePiece(1); }
        else if (IsKeyPressed(KEY_Z)) { rotatePiece(-1); }

        // hold pieces
        if (IsKeyPressed(KEY_C) && canHold) { holdPiece(); canHold = 0; }

        // moving pieces
        if (dt >= 0.065f) { 
            if (IsKeyDown(KEY_LEFT)) { movePiece(-1); }
            else if (IsKeyDown(KEY_RIGHT)) { movePiece(1); }
            if (IsKeyDown(KEY_DOWN)) { updateBoard(); }
            dt = 0.0f;
        }

        // Draw
        BeginDrawing();

            ClearBackground(BLACK);
            drawBoard();
            level::drawLinesCleared();
            level::drawScore();
            level::drawLevel();
            piece::drawNextPiece();
            piece::drawHeldPiece();

        EndDrawing();

        // hard drop
        if (IsKeyPressed(KEY_SPACE)) { hardDropPiece(); }

        // Gravity
        if (grav >= level::getTime()) {
            updateBoard();
            grav = 0.0f;
        }

    } else if (gameState == gameOverState) { // game over screen
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { gameState = menuState; resetBoard(); }

        BeginDrawing();

            ClearBackground(BLACK);

            int textWidth = MeasureText("Game Over", 10 * fontScale);
            DrawText("Game Over", screenWidth/2 - textWidth/2, screenHeight/2 - 7.5 * fontScale, 10 * fontScale, WHITE);

            level::displayScore();

        EndDrawing();
    }
}

// does a cell have a piece
static bool hasPiece(const int row, const int col) {
    // check if out of bunds -- if it is say there is a piece there to prevent movement to said cell
    if (row < 0 || row >= boardHeight || col < 0 || col >= boardWidth) { return 1; }
    if (isActive(row, col)) { return 0; }

    Color color = board[row][col];
    return color.r || color.g || color.b;
}

// does a cell contain an active piece
static bool isActive(const int row, const int col) {
    for (int i = 0; i < piece::numPoints; i++) {
        if (activePiece[i][0] == row && activePiece[i][1] == col) { return true; }
    }

    return false;
}

// check if all components of the piece can move a specified amount
static bool canMove(int cords[4][2], const int rowMod, const int colMod) {
    for (int i = 0; i < piece::numPoints; i++) {
        if (hasPiece(cords[i][0] + rowMod, cords[i][1] + colMod)) { return 0; }
    }

    return 1;
}

// update the piece's rotation
static bool updatePiece(int cords[4][2], const int row, const int col, const bool vert, const int dir, int& mod) {
    // move the piece vertically
    if (vert) {
        for (int i = 1; i <= maxBuffer; i++) {
            if (!hasPiece(row + i, col) && canMove(cords, i, 0)) {
                mod = i;
                return 1;
            }

            // oscilation
            if (i > 0) { i -= 2*i + 1; }
            else { i -= 2*i; }
        }

        // no valid moves
        return 0;
    }

    // move the piece horizontally
    // determine the direction to move horizontally in
    for (int i = 1; i <= maxBuffer; i++) {
        if (!hasPiece(row, col - (i * dir)) && canMove(cords, 0, -i * dir)) {
            mod = -i * dir;
            return 1;
        }
    }

    // no valid moves
    return 0;
}

// lose condition
static bool hasLost() {
    return hasPiece(0, boardWidth/2 - 2) || hasPiece(0, boardWidth/2 - 1) || hasPiece(0, boardWidth/2 + 1) || 
            hasPiece(0, boardWidth/2 + 2) || isActive(0, boardWidth/2 - 2) || isActive(0, boardWidth/2 - 1 || 
            isActive(0, boardWidth/2 + 1) || isActive(0, boardWidth/2 + 2)) || hasPiece(1, boardWidth/2 - 1) || 
            hasPiece(1, boardWidth/2 + 1) || isActive(1, boardWidth/2 - 1) || isActive(1, boardWidth/2 + 1);
}

// drop pieces
static void hardDropPiece() {
    int count = 0; // used to ensure all parts of the active piece can fall
    bool check = 1; // used to loop through to hard drop

    // drop the active piece
    while (check) {
        int prevCount = count; // ensures flat pieces don't loop indefinitely

        for (int i = 0; i < piece::numPoints; i++) {
            if (!hasPiece(activePiece[i][0] + count/4, activePiece[i][1])) { count++; }
        }

        if (count - prevCount < 4) { check = 0; }
    }

    level::updateScore(count/4, 1);

    // used for independence from index order
    Color color = board[activePiece[0][0]][activePiece[0][1]];

    // update position
    for (int i = 0; i < piece::numPoints; i++) { board[activePiece[i][0]][activePiece[i][1]] = BLACK; }
    for (int i = 0; i < piece::numPoints; i++) {
        activePiece[i][0] += count/4 - 1;
        board[activePiece[i][0]][activePiece[i][1]] = color;
    }

    // clear completed lines -- only check if the active piece has been placed
    clearLines();
    if (hasLost()) {
        gameState = gameOverState;
        return;
    }

    canHold = 1;

    // drop new piece and set it equal to the active piece if the current piece is unable to move
    pieceType = piece::getPiece();
    Color pieceColor = piece::getColor();
    int** points = piece::getPoints();

    for (int i = 0; i < piece::numPoints; i++) {
        board[points[i][0]][points[i][1]] = pieceColor;
        activePiece[i][0] = points[i][0];
        activePiece[i][1] = points[i][1];
        delete[] points[i];
    }

    delete[] points;
}

// clear possible lines
static void clearLines() {
    int count = 0, numLines = 0;

    for (int i = 0; i < boardHeight; i++) {
        for (int j = 0; j < boardWidth; j++) {
            if (hasPiece(i, j) || isActive(i, j)) { count++; }
        }

        if (count == 10) {
            numLines++;
            for (int j = 0; j < boardWidth; j++) { board[i][j] = BLACK; } // remove all pieces in that line

            // move each row above the completed line down
            for (int r = i - 1; r >= 0; r--) {
                for (int c = 0; c < boardWidth; c++) {
                    board[r + 1][c] = board[r][c];
                    board[r][c] = BLACK;
                }
            }
        }

        count = 0;
    }

    if (numLines) { level::updateScore(numLines, 0); }
 }

// move left and right -- 1 = right, -1 = left
static void movePiece(const int dir) {
    int count = 0; // ensure all parts of the piece can move left or right

    for (int i = 0; i < piece::numPoints; i++) {
        int row = activePiece[i][0], col = activePiece[i][1];
        bool inBounds = ((col < boardWidth - 1 && dir == 1) || (col > 0 && dir == -1));
        if (isActive(row, col + dir) || (inBounds && !hasPiece(row, col + dir))) { count++; }
    }

    // ensure all active pieces can move
    if (count != 4) { return; }

    // use for independence from index order
    Color color = board[activePiece[0][0]][activePiece[0][1]];

    // update positions
    for (int i = 0; i < piece::numPoints; i++) { board[activePiece[i][0]][activePiece[i][1]] = BLACK; }
    for (int i = 0; i < piece::numPoints; i++) {
        activePiece[i][1] += dir;
        board[activePiece[i][0]][activePiece[i][1]] = color;
    }
}

// force the piece to stay in bounds when rotating
static void forceInBounds(int cords[4][2], const int dir) {
    int rowModifier = 0, colModifier = 0;
    int addRow = 0, addCol = 0;

    // determine the modifiers
    for (int i = 0; i < piece::numPoints; i++) {
        int row = cords[i][0], col = cords[i][1];

        if (row < 0) { rowModifier = std::max(rowModifier, -1 * row); }
        if (row >= boardHeight) { rowModifier = std::min(rowModifier, -1 * (row - (boardHeight - 1))); }
        if (col < 0) { colModifier = std::max(colModifier, -1 * col); }
        if (col >= boardWidth) { colModifier = std::min(colModifier, -1 * (col - (boardWidth - 1))); }
    }

    // modify all active pieces by the modifiers
    for (int i = 0; i < piece::numPoints; i++) {
        cords[i][0] += rowModifier;
        cords[i][1] += colModifier;
    }

    // ensure piece is not within another
    for (int i = 0; i < piece::numPoints; i++) {
        if (hasPiece(cords[i][0], cords[i][1])) {
            if (!updatePiece(cords, cords[i][0], cords[i][1], 1, dir, addRow)) {
                // pushed from outbounds
                if (colModifier) {
                    cords[0][0] = 0;
                    cords[0][1] = 0;
                    cords[1][0] = 0;
                    cords[1][1] = 0;
                    return;
                }

                // horizontal check
                if (!updatePiece(cords, cords[i][0], cords[i][1], 0, dir, addCol)) {
                    cords[0][0] = 0;
                    cords[0][1] = 0;
                    cords[1][0] = 0;
                    cords[1][1] = 0;
                    return;
                }

                break;
            }

            break;
        }
    }

    // modify each value
    for (int i = 0; i < piece::numPoints; i++) {
        cords[i][0] += addRow;
        cords[i][1] += addCol;
    }
}

// rotates the active piece either left or right
// left is denotated by -1 and right by 1
static void rotatePiece(const int dir) {
    // convert to local cords based around the pivot point of the piece
    int originRow = activePiece[3][0], originCol = activePiece[3][1];
    Color originalColor = board[activePiece[0][0]][activePiece[0][1]];

    int localCords[3][2] = {
        {activePiece[0][0] - originRow, activePiece[0][1] - originCol},
        {activePiece[1][0] - originRow, activePiece[1][1] - originCol},
        {activePiece[2][0] - originRow, activePiece[2][1] - originCol}
    };

    // transform the local cords using L((r, c)) = (c, -r) * dir
    for (int i = 0; i < 3; i++) {
        int temp = localCords[i][0];
        localCords[i][0] = localCords[i][1] * dir;
        localCords[i][1] = -1 * temp * dir;
    }

    // update the active piece based on the transformation
    int newLoc[4][2];

    for (int i = 0; i < 3; i++) {
        newLoc[i][0] = localCords[i][0] + originRow;
        newLoc[i][1] = localCords[i][1] + originCol;
    }

    newLoc[3][0] = activePiece[3][0];
    newLoc[3][1] = activePiece[3][1];

    // check if rotation isn't allowed
    forceInBounds(newLoc, dir);
    if (!newLoc[0][0] && !newLoc[0][1] && !newLoc[1][0] && !newLoc[1][1]) { return; }

    // update colors
    for (int i = 0; i < piece::numPoints; i++) { board[activePiece[i][0]][activePiece[i][1]] = BLACK; }

    for (int i = 0; i < piece::numPoints; i++) {
        activePiece[i][0] = newLoc[i][0];
        activePiece[i][1] = newLoc[i][1];
        board[activePiece[i][0]][activePiece[i][1]] = originalColor;
    }
}

// hold a piece
static void holdPiece() {
    // clear the piece from the board
    for (int i = 0; i < piece::numPoints; i++) { board[activePiece[i][0]][activePiece[i][1]] = BLACK; }

    int** points = piece::holdPiece(pieceType);
    
    if (!points[0][0] && !points[0][1] && !points[1][0] && !points[1][1]) {
        pieceType = piece::getPiece();
        Color color = piece::getColor();
        int** points = piece::getPoints();

        for (int i = 0; i < piece::numPoints; i++) {
            activePiece[i][0] = points[i][0];
            activePiece[i][1] = points[i][1];
            board[activePiece[i][0]][activePiece[i][1]] = color;
            delete[] points[i];
        }
        
        return;
    }

    Color color = piece::getColor(pieceType);

    for (int i = 0; i < piece::numPoints; i++) {
        activePiece[i][0] = points[i][0];
        activePiece[i][1] = points[i][1];
        board[activePiece[i][0]][activePiece[i][1]] = color;
        delete[] points[i];
    }

    delete[] points;
}

// update the tetris board -- gravity
static void updateBoard() {
    int count = 0; // used to ensure all parts of the active piece can fall

    // update the active piece
    for (int i = 0; i < piece::numPoints; i++) {
        int row = activePiece[i][0], col = activePiece[i][1];
        if (isActive(row + 1, col) || (row < boardHeight - 1 && !hasPiece(row + 1, col))) { count++; }
    }

    if (count == 4) {
        // used for independence from index order
        Color color = board[activePiece[0][0]][activePiece[0][1]];

        // update position
        for (int i = 0; i < piece::numPoints; i++) { board[activePiece[i][0]][activePiece[i][1]] = BLACK; }
        for (int i = 0; i < piece::numPoints; i++) {
            activePiece[i][0]++;
            board[activePiece[i][0]][activePiece[i][1]] = color;
        }

        return;
    }

    // clear completed lines -- only check if the active piece has been placed
    clearLines();
    if (hasLost()) {
        gameState = gameOverState;
        return;
    }

    canHold = 1;

    // drop new piece and set it equal to the active piece if the current piece is unable to move
    pieceType = piece::getPiece();
    Color color = piece::getColor();
    int** points = piece::getPoints();

    for (int i = 0; i < piece::numPoints; i++) {
        board[points[i][0]][points[i][1]] = color;
        activePiece[i][0] = points[i][0];
        activePiece[i][1] = points[i][1];
        delete[] points[i];
    }

    delete[] points;
}

// draw the tetris board
static void drawBoard() {
    // draw tiles and boards in one
    for (int r = 0; r < boardHeight; r++) {
        DrawLine(offsetX, offsetY + tileSize*r, offsetX + tileSize*boardWidth, offsetY + tileSize*r, WHITE);

        for (int c = 0; c < boardWidth; c++) {
            // only draw if there's a piece
            if (isActive(r, c) || hasPiece(r, c)) {
                DrawRectangle(offsetX + tileSize*c, offsetY + tileSize*r, tileSize, tileSize, board[r][c]);
            }

            DrawLine(offsetX + tileSize*c, offsetY, offsetX + tileSize*c, offsetY + tileSize*boardHeight, WHITE);
        }
    }

    DrawLine(offsetX, offsetY + tileSize*boardHeight, offsetX + tileSize*boardWidth, offsetY + tileSize*boardHeight, WHITE);
    DrawLine(offsetX + tileSize*boardWidth, offsetY, offsetX + tileSize*boardWidth, offsetY + tileSize*boardHeight, WHITE);
}

static void resetBoard() {
    startingPiece = 1;

    // reset the board
    for (int i = 0; i < boardHeight; i++) {
        for (int j = 0; j < boardWidth; j++) {
            board[i][j] = BLACK;
        }
    }

    // reset score, lines cleared, heldPiece, and the level
    level::reset();
    piece::reset();
}
