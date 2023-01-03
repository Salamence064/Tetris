#ifndef tetrisPiece
#define tetrisPiece

#include <algorithm>
#include <random>
#include <vector>
#include "raylib.h"

// operators involving pieces
namespace piece {
    // private members
    namespace {
        const int indices[7][4][2] = {
            {{0, 3}, {0, 4}, {0, 5}, {0, 6}},
            {{0, 4}, {0, 5}, {1, 4}, {1, 5}},
            {{0, 4}, {1, 3}, {1, 4}, {1, 5}},
            {{0, 4}, {0, 5}, {1, 3}, {1, 4}},
            {{0, 3}, {0, 4}, {1, 4}, {1, 5}},
            {{0, 3}, {1, 3}, {1, 4}, {1, 5}},
            {{0, 5}, {1, 3}, {1, 4}, {1, 5}}
        };

        const Color colors[7] = {
            (Color) {0, 255, 255, 255},
            (Color) {255, 255, 0, 255},
            (Color) {128, 0, 128, 255},
            (Color) {0, 255, 0, 255},
            (Color) {255, 0, 0, 255},
            (Color) {0, 0, 255, 255},
            (Color) {255, 127, 0, 255}
        };

        std::vector<int> pieceOrder = {0, 1, 2, 3, 4, 5, 6};
        int currIndex = 6; // current index for the piece to retrieve from pieceOrder
        bool update = 1; // used for scrambling the pieceOrder
        int heldPiece = -1; // keep track of the currently held piece: -1 == no held piece
    }

    // public attributes
    const int numPoints = 4;

    // public functions
    Color getColor() { return colors[pieceOrder[currIndex]]; };

    // overloaded to allow for proper color assignment for held pieces
    Color getColor(const int p) { return colors[p]; };

    int** getPoints() {
        int** points = new int*[4];

        for (int i = 0; i < numPoints; i++) {
            // add each point
            points[i] = new int[2];
            for (int j = 0; j < 2; j++) { points[i][j] = indices[pieceOrder[currIndex]][i][j]; }
        }

        currIndex++;
        if (currIndex > 6) { currIndex = 0; update = 1; }
        return points;
    };

    // get the piece to drop
    // used to determine the piece for held pieces
    int getPiece() { return pieceOrder[currIndex]; };

    int** holdPiece(int& type) {
        int** points = new int*[4];

        if (heldPiece < 0) {
            for (int i = 0; i < numPoints; i++) {
                points[i] = new int[2];
                points[i][0] = 0;
                points[i][1] = 0;
            }

            heldPiece = type;
            return points;
        }

        for (int i = 0; i < numPoints; i++) {
            points[i] = new int[2];
            for (int j = 0; j < 2; j++) { points[i][j] = indices[heldPiece][i][j]; }
        }

        int temp = heldPiece;
        heldPiece = type;
        type = temp;

        return points;
    };

    void reset() { heldPiece = -1; };

    void drawHeldPiece();
    void drawNextPiece();
}

#endif // ! tetrisPiece