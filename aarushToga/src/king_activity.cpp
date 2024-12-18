#include "board.h"
#include "util.h"

// King activity scoring function
int evaluate_king_activity(const board_t *board, int colour) {
    int king = KING_POS(board, colour); // King's current position
    int phase = board->phase;          // Game phase
    int score = 0;

    if (phase < 128) { // Endgame phase
        score += 14 - DISTANCE(king, CENTER_SQUARE); // Bonus for central king
    } else { // Opening and Middlegame
        if (SQUARE_FILE(king) == FileA || SQUARE_FILE(king) == FileH) {
            score -= 20; // Penalty for king on the edge
        }
        if (SQUARE_RANK(king) == Rank1 || SQUARE_RANK(king) == Rank8) {
            score -= 10; // Penalty for king on the back rank
        }
    }

    return score;
}
