#include "bitboard.h"
#include "piece.h"
#include <cstring>
#include "math.h"

namespace BitboardUtils {
    // Precomputed attack tables (global storage)
    Bitboard KNIGHT_ATTACKS[64];
    Bitboard KING_ATTACKS[64];
    Bitboard PAWN_ATTACKS[2][64];
    
    Bitboard DIAGONAL_MASK[64];
    Bitboard HORIZONTAL_MASK[64];
    Bitboard VERTICAL_MASK[64];
    
    Bitboard DIAGONAL_ATTACKS[64][512];
    Bitboard HORIZONTAL_ATTACKS[64][256];
    Bitboard VERTICAL_ATTACKS[64][256];

    // Mask generation functions
    Bitboard generateDiagonalMask(int square) {
        Bitboard mask = 0;
        int rank = square / 8;
        int file = square % 8;
        
        for (int r = 0; r < 8; r++) {
            for (int f = 0; f < 8; f++) {
                if (abs(r - rank) == abs(f - file)) {
                    mask |= (1ULL << (r * 8 + f));
                }
            }
        }
        
        return mask;
    }

    Bitboard generateHorizontalMask(int square) {
        int rank = square / 8;
        return 0xFFULL << (rank * 8);
    }

    Bitboard generateVerticalMask(int square) {
        int file = square % 8;
        Bitboard mask = 0;
        for (int r = 0; r < 8; r++) {
            mask |= (1ULL << (r * 8 + file));
        }
        return mask;
    }

    // Generate knight attack patterns
    void initializeKnightAttacks() {
        const int knightMoves[8][2] = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
            {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };

        for (int sq = 0; sq < 64; sq++) {
            Bitboard attacks = 0;
            int rank = sq / 8;
            int file = sq % 8;

            for (auto& move : knightMoves) {
                int newRank = rank + move[0];
                int newFile = file + move[1];

                if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                    int newSq = newRank * 8 + newFile;
                    attacks |= (1ULL << newSq);
                }
            }
            KNIGHT_ATTACKS[sq] = attacks;
        }
    }

    // Generate king attack patterns
    void initializeKingAttacks() {
        const int kingMoves[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            {0, -1}, {0, 1},
            {1, -1}, {1, 0}, {1, 1}
        };

        for (int sq = 0; sq < 64; sq++) {
            Bitboard attacks = 0;
            int rank = sq / 8;
            int file = sq % 8;

            for (auto& move : kingMoves) {
                int newRank = rank + move[0];
                int newFile = file + move[1];

                if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                    int newSq = newRank * 8 + newFile;
                    attacks |= (1ULL << newSq);
                }
            }
            KING_ATTACKS[sq] = attacks;
        }
    }

    // Generate pawn attack patterns
    void initializePawnAttacks() {
        for (int sq = 0; sq < 64; sq++) {
            int rank = sq / 8;
            int file = sq % 8;

            // White pawn attacks
            Bitboard whiteAttacks = 0;
            if (rank < 7) {
                if (file > 0) whiteAttacks |= (1ULL << (sq + 7));
                if (file < 7) whiteAttacks |= (1ULL << (sq + 9));
            }
            PAWN_ATTACKS[0][sq] = whiteAttacks;

            // Black pawn attacks
            Bitboard blackAttacks = 0;
            if (rank > 0) {
                if (file > 0) blackAttacks |= (1ULL << (sq - 9));
                if (file < 7) blackAttacks |= (1ULL << (sq - 7));
            }
            PAWN_ATTACKS[1][sq] = blackAttacks;
        }
    }

    // Generate slider attack tables (simplified example)
    void initializeSliderAttacks() {
        for (int sq = 0; sq < 64; sq++) {
            // Diagonal attacks
            DIAGONAL_MASK[sq] = generateDiagonalMask(sq);
            
            // Horizontal attacks
            HORIZONTAL_MASK[sq] = generateHorizontalMask(sq);
            
            // Vertical attacks
            VERTICAL_MASK[sq] = generateVerticalMask(sq);
        }
    }

    // Main initialization function
    void initializeBitboardAttacks() {
        initializeKnightAttacks();
        initializeKingAttacks();
        initializePawnAttacks();
        initializeSliderAttacks();
    }

    // Advanced slider attack calculation
    Bitboard getSliderAttacks(int piece, int square, Bitboard occupancy) {
        // Implement advanced attack generation based on piece type and occupancy
        switch(piece) {
            case Bishop64: {
                // Use diagonal mask and occupancy to generate attacks
                Bitboard diagonalOccupancy = occupancy & DIAGONAL_MASK[square];
                // Advanced magic bitboard calculation would go here
                return 0; // Placeholder
            }
            case Rook64: {
                // Use horizontal and vertical masks
                Bitboard horizontalOccupancy = occupancy & HORIZONTAL_MASK[square];
                Bitboard verticalOccupancy = occupancy & VERTICAL_MASK[square];
                // Advanced magic bitboard calculation would go here
                return 0; // Placeholder
            }
            case Queen64: {
                // Combine bishop and rook attacks
                return getSliderAttacks(Bishop64, square, occupancy) | 
                       getSliderAttacks(Rook64, square, occupancy);
            }
            default:
                return 0;
        }
    }

    // Utility functions
    int popCount(Bitboard b) {
        return __builtin_popcountll(b);  // GCC/Clang intrinsic
    }

    int bitScanForward(Bitboard b) {
        return __builtin_ctzll(b);  // GCC/Clang intrinsic
    }
}