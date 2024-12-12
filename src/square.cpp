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

#include "square.h"

const std::string SquareString[65] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "xx"
};

std::string square_to_string(const Square sq){ return SquareString[sq]; }

Square string_to_sq(const std::string s){
    int f=-1, r=-1;
    switch (s[0]){
        case 'a':{f=0; break;}
        case 'b':{f=1; break;}
        case 'c':{f=2; break;}
        case 'd':{f=3; break;}
        case 'e':{f=4; break;}
        case 'f':{f=5; break;}
        case 'g':{f=6; break;}
        case 'h':{f=7; break;}
    };
    switch (s[1]){
        case '1':{r=0; break;}
        case '2':{r=1; break;}
        case '3':{r=2; break;}
        case '4':{r=3; break;}
        case '5':{r=4; break;}
        case '6':{r=5; break;}
        case '7':{r=6; break;}
        case '8':{r=7; break;}
    };
    return Square((r*8)+f);
}

