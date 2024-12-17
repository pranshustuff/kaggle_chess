#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>

using Bitboard = uint64_t;

namespace BitboardUtils {
    // Precomputed attack tables declarations
    extern Bitboard KNIGHT_ATTACKS[64];
    extern Bitboard KING_ATTACKS[64];
    extern Bitboard PAWN_ATTACKS[2][64];
    
    extern Bitboard DIAGONAL_MASK[64];
    extern Bitboard HORIZONTAL_MASK[64];
    extern Bitboard VERTICAL_MASK[64];
    
    extern Bitboard DIAGONAL_ATTACKS[64][512];
    extern Bitboard HORIZONTAL_ATTACKS[64][256];
    extern Bitboard VERTICAL_ATTACKS[64][256];

    // Initialization functions
    void initializeBitboardAttacks();
    
    // Mask generation functions
    Bitboard generateDiagonalMask(int square);
    Bitboard generateHorizontalMask(int square);
    Bitboard generateVerticalMask(int square);

    // Attack calculation functions
    Bitboard getSliderAttacks(int piece, int square, Bitboard occupancy);
    
    // Utility functions
    int popCount(Bitboard b);
    int bitScanForward(Bitboard b);
}

#endif