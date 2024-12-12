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

#ifndef MOVE_H
#define MOVE_H

#include "square.h"

/**
  * Move:
  * 0000000000001111 - type
  * 0000001111110000 - from (0 - 63)
  * 1111110000000000 - to (0 - 63)
  */

typedef int Move;

#define NullMove 0
#define MoveQuiet 1
#define MoveCapture 2
#define MoveCastle 3
#define MovePN 4
#define MovePB 6
#define MovePR 8
#define MovePQ 10
#define MoveEP 11
#define NO_Move 12

typedef int Castle;

#define N0_CASTLE 0
#define WK_CASTLE 1
#define WQ_CASTLE 2
#define W_CASTLE 3
#define BK_CASTLE 4
#define BQ_CASTLE 8
#define B_CASTLE 12

const int RELATIVE_CASTLE[2][4] = {
    {N0_CASTLE, WK_CASTLE, WQ_CASTLE, W_CASTLE},
    {N0_CASTLE ,BK_CASTLE, BQ_CASTLE, B_CASTLE}
};

inline Square move_from(const Move m) {
    return Square((int(m) >> 4) & 0x3F);
}

inline Square move_to(const Move m) {
    return Square((int(m) >> 10) & 0x3F);
}

inline Move move_type(const Move m) {
    return Move(m & 15);
}

inline Move move_create(const Move m, Square from, Square to){
    return m | from << 4 | to << 10;
}

inline bool is_tactical(const Move move){
    int type = move_type(move);
    return (type == MoveCapture || type >= MovePN);
}

inline bool is_castle(const Move move){
    return move_type(move) == MoveCastle;
}

std::string move_to_string(const Move m);

#endif // MOVE_H
