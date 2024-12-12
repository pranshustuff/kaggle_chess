/***************************************************************************
 *   Copyright (C) 2009-2010 by Borko Boskovic                             *
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

#ifndef SQUARE_H
#define SQUARE_H

#include <cstdlib>
#include <algorithm>
#include <string>

typedef int Square;
#define A1 0
#define B1 1
#define C1 2
#define D1 3
#define E1 4
#define F1 5
#define G1 6
#define H1 7
#define A2 8
#define B2 9
#define C2 10
#define D2 11
#define E2 12
#define F2 13
#define G2 14
#define H2 15
#define A3 16
#define B3 17
#define C3 18
#define D3 19
#define E3 20
#define F3 21
#define G3 22
#define H3 23
#define A4 24
#define B4 25
#define C4 26
#define D4 27
#define E4 28
#define F4 29
#define G4 30
#define H4 31
#define A5 32
#define B5 33
#define C5 34
#define D5 35
#define E5 36
#define F5 37
#define G5 38
#define H5 39
#define A6 40
#define B6 41
#define C6 42
#define D6 43
#define E6 44
#define F6 45
#define G6 46
#define H6 47
#define A7 48
#define B7 49
#define C7 50
#define D7 51
#define E7 52
#define F7 53
#define G7 54
#define H7 55
#define A8 56
#define B8 57
#define C8 58
#define D8 59
#define E8 60
#define F8 61
#define G8 62
#define H8 63
#define NO_SQ 64

typedef int File;
#define FILE_A 0
#define FILE_B 1
#define FILE_C 2
#define FILE_D 3
#define FILE_E 4
#define FILE_F 5
#define FILE_G 6
#define FILE_H 7
#define FILE_NONE 8

typedef int Rank;
#define RANK_1 0
#define RANK_2 1
#define RANK_3 2
#define RANK_4 3
#define RANK_5 4
#define RANK_6 5
#define RANK_7 6
#define RANK_8 7
#define RANK_NONE 8

const int RELATIVE_RANK[2][8] = {
    { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8},
    { RANK_8, RANK_7, RANK_6, RANK_5, RANK_4, RANK_3, RANK_2, RANK_1}
};

typedef int Color;
#define White 0
#define Black 1

inline Rank rank(const Square sq){
    return Rank(sq>>3);
}

inline File file(const Square sq){
    return File(sq&7);
}

inline Color color(const Square sq){
    return Color(((sq^(sq>>3))&1)^1);
}

inline Square opposite(const Square sq){
    return Square(sq^070);
}

inline Square get_square(const File f, const Rank r) {
    return Square((r<<3)|f);
}

inline Square rank_mirror_sq(const Square sq) {
    return Square(sq^0x38);
}

inline Square file_mirror_sq(const Square sq) {
    return Square(sq^0x7);
}

inline Square relative_sq(const Color c, const Square sq){
    if(c == White) return sq;
    else return rank_mirror_sq(sq);
}

template <Color c> inline Square relative_sq(const Square sq){
    return relative_sq(c,sq);
}

template <Color c> inline Square relative_rank(const Rank r){
    if(c == White) return r;
    else return RANK_8-r;
}

inline int distance(const Square from, const Square to){
    int dist1 = abs(file(from) - file(to));
    int dist2 = abs(rank(from) - rank(to));
    return std::max(dist1,dist2);
}

inline int corner_distance(Square sq){
    File f = file(sq);
    Rank r = rank(sq);
    if(f >= FILE_E) f = File(FILE_H - f);
    if(r >= RANK_5) r = Rank(RANK_8 - r);
    return std::max(int(f),int(r));
}

template<Color c> inline int relative_add(int r, int x){
    if(c == White) return r+x;
    else return r-x;
}

template<Color c> inline int relative_value(int x){
    if(c == White) return x;
    else return -x;
}

std::string square_to_string(const Square sq);
Square string_to_sq(const std::string s);

#endif // SQUARE_H
