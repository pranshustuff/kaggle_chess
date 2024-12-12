/***************************************************************************
 *   Copyright (C) 2009-2011 by Borko Boskovic                             *
 *   borko.boskovic@gmail.com                                              *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef PIECE_H
#define PIECE_H

#include "square.h"

typedef int  Piece;
#define W 0
#define B 1
#define WP 2
#define BP 3
#define WN 4
#define BN 5
#define WB 6
#define BB 7
#define WR 8
#define BR 9
#define WQ 10
#define BQ 11
#define WK 12
#define BK 13
#define NO_PIECE 14

typedef int PieceType;
#define PAWN 2
#define KNIGHT 4
#define BISHOP 6
#define ROOK 8
#define QUEEN 10
#define KING 12

#define PAWN_VALUE 80
#define KNIGHT_VALUE 370
#define BISHOP_VALUE 372
#define ROOK_VALUE 570
#define QUEEN_VALUE 1120
#define KING_VALUE 10000

#define EG_PAWN_VALUE 104
#define EG_KNIGHT_VALUE 362
#define EG_BISHOP_VALUE 364
#define EG_ROOK_VALUE 580
#define EG_QUEEN_VALUE 1160
#define EG_KING_VALUE 10000

#define SSE_GOOD (2*(KNIGHT_VALUE - BISHOP_VALUE))

const int MAT[12][2] = {
// WHITE BLACK
// O,            E                 O,           E
  {0,            0},               {0,           0},
  {PAWN_VALUE,   EG_PAWN_VALUE},   {PAWN_VALUE,   EG_PAWN_VALUE},   // PAWN
  {KNIGHT_VALUE, EG_KNIGHT_VALUE}, {KNIGHT_VALUE, EG_KNIGHT_VALUE}, // KNIGHT
  {BISHOP_VALUE, EG_BISHOP_VALUE}, {BISHOP_VALUE, EG_BISHOP_VALUE}, // BISHOP
  {ROOK_VALUE,   EG_ROOK_VALUE},   {ROOK_VALUE,   EG_ROOK_VALUE},   // ROOK
  {QUEEN_VALUE,  EG_QUEEN_VALUE},  {QUEEN_VALUE,  EG_QUEEN_VALUE}   // QUEEN
};

const int PieceValue[15] = {
    0, 0,
    PAWN_VALUE, PAWN_VALUE,
    KNIGHT_VALUE, KNIGHT_VALUE,
    BISHOP_VALUE, BISHOP_VALUE,
    ROOK_VALUE, ROOK_VALUE,
    QUEEN_VALUE, QUEEN_VALUE,
    KING_VALUE, KING_VALUE,
    0
};

inline int piece_value(const Piece p){
    return PieceValue[p];
}

inline Color opponent (const Color c) {
    return (c ^ 1);
}

char piece_to_char(const Piece p);

Piece char_to_piece(const char c);

#endif // PIECE_H
