#ifndef levels
#define levels

// operators involving levels, score, and the number of cleared lines
namespace level {
    // private attributes
    namespace {
        int score = 0;
        int linesCleared = 0;
        int level = 1;

        // base scores determined by the lines cleared
        int scores[4] = {40, 100, 400, 1200};
    }

    // public functions
    void drawScore();
    void drawLinesCleared();
    void drawLevel();
    void displayScore(); // used for game over screen

    // update the score -- bool tells the function to handle it as a harddrop or line clear
    void updateScore(const int numLines, const bool hardDrop) {
        if (hardDrop) {
            score += numLines;
            return;
        }

        linesCleared += numLines;
        score += scores[numLines - 1];

        // check to level up
        if (level * 10 <= linesCleared) { level++; }
    };

    // time it takes for the piece to drop one row in seconds
    float getTime() {
        if (1 <= level && level <= 10) { return 0.275f - 0.025f * level; }
        return 0.015f;
    };
}

#endif // ! levels