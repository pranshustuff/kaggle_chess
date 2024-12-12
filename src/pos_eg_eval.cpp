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

#include "position.h"

bool Position::eg_eval(int & eval) const {
    int p = num_pieces();
    int m = abs(eval_mat());
    switch(p){
        case 2:{ eval = DRAW; return true;}                             // KK
        case 3:{
            switch(m){
                case KNIGHT_VALUE:  { eval = DRAW; return true; }       // KNK
                case BISHOP_VALUE:  { eval = DRAW; return true; }       // KBK
                case ROOK_VALUE:    { eval = KRK(); return true; }      // KRK
                case QUEEN_VALUE:   { eval = KQK(); return true; }      // KQK
                case PAWN_VALUE:    {                                   // KPK
                    if(draw_KPK()){ eval = DRAW; return true; }
                    return false;
                }
            }
            break;
        }
        case 4:{
            switch(m){
                case 0:{
                    if(bitboard[WN] != 0){ eval = DRAW; return true; }  // KNKN
                    if(bitboard[WB] != 0){ eval = DRAW; return true; }  // KBKB
                    if(bitboard[WR] != 0){ eval = DRAW; return true; }  // KRKR
                    if(bitboard[WQ] != 0){ eval = DRAW; return true; }  // KQKQ
                    break;
                }
                case (2*KNIGHT_VALUE): { eval = DRAW; return true; }    // KNNK
                case (2*BISHOP_VALUE): { eval = KBBK(); return true; }  // KBBK
                case (BISHOP_VALUE+KNIGHT_VALUE):                       // KBNK
                    { eval = KBNK(); return true; }
                case (KNIGHT_VALUE-BISHOP_VALUE):                       // KBKN
                    { eval = DRAW; return true; }
                case(2*ROOK_VALUE): { eval = KRRK(); return true; }     // KRRK
                case(2*QUEEN_VALUE): { eval = KQQK(); return true; }    // KQQK
                case(ROOK_VALUE+QUEEN_VALUE):                           // KQRK
                    { eval = KQRK(); return true; }
                case BISHOP_VALUE+PAWN_VALUE:                           // KBPK
                    { if(draw_KBPK()){ eval = DRAW; return true; } }
            }
            break;
        }
    }
    return false;
}

bool Position::eg_eval(int & eval, EvalInfo & info) const {
    int p = num_pieces();
    int m = abs(eval_mat());

    switch(p){
        case 2:{ eval = DRAW; return true;}                             // KK
        case 3:{
            switch(m){
                case KNIGHT_VALUE:  { eval = DRAW; return true; }       // KNK
                case BISHOP_VALUE:  { eval = DRAW; return true; }       // KBK
                case ROOK_VALUE:    { eval = KRK(); return true; }      // KRK
                case QUEEN_VALUE:   { eval = KQK(); return true; }      // KQK
                case PAWN_VALUE:    {                                   // KPK
                    if(draw_KPK()){ eval = DRAW; return true; }
                    return false;
                }
            }
            break;
        }
        case 4:{
            switch(m){
                case 0:{
                    if(bitboard[WN] != 0){ eval = DRAW; return true; }  // KNKN
                    if(bitboard[WB] != 0){ eval = DRAW; return true; }  // KBKB
                    if(bitboard[WR] != 0){ eval = DRAW; return true; }  // KRKR
                    if(bitboard[WQ] != 0){ eval = DRAW; return true; }  // KQKQ
                    break;
                }
                case (2*KNIGHT_VALUE): { eval = DRAW; return true; }    // KNNK
                case (2*BISHOP_VALUE): { eval = KBBK(); return true; }  // KBBK
                case (BISHOP_VALUE+KNIGHT_VALUE):                       // KBNK
                    { eval = KBNK(); return true; }
                case (KNIGHT_VALUE-BISHOP_VALUE):                       // KBKN
                    { eval = DRAW; return true; }
                case(2*ROOK_VALUE): { eval = KRRK(); return true; }     // KRRK
                case(2*QUEEN_VALUE): { eval = KQQK(); return true; }    // KQQK
                case(ROOK_VALUE+QUEEN_VALUE):                           // KQRK
                    { eval = KQRK(); return true; }
                case(QUEEN_VALUE-PAWN_VALUE):                           // KQKP
                    { draw_KQKP(info.mul); return false; }
                case(ROOK_VALUE-PAWN_VALUE):                            // KRKP
                    { draw_KRKP(info.mul); return false; }
                case(BISHOP_VALUE-PAWN_VALUE):                          // KBKP
                    { draw_KBKP(info.mul); return false; }
                case(KNIGHT_VALUE-PAWN_VALUE):                          // KNKP
                    { draw_KNKP(info.mul); return false; }
                case(KNIGHT_VALUE+PAWN_VALUE):                          // KNPK
                    { draw_KNPK(info.mul); return false;}
                case BISHOP_VALUE+PAWN_VALUE:                           // KBPK
                    { if(draw_KBPK()){ eval = DRAW; return true; } }
            }
            break;
        }
        case 5:{
            switch(m){
                case PAWN_VALUE:{
                    if(bitboard[ROOK+White]&&bitboard[ROOK+Black]){     //KRPKR
                        draw_KRPKR(info.mul); return false;
                    }
                    if(bitboard[BISHOP+White]&&bitboard[BISHOP+Black]){ //KBPKB
                        draw_KBPKB(info.mul); return false;
                    }
                }
            };
        }
    }
    return false;
}

bool Position::eg_draw() const {
    int p = num_pieces();
    int m = abs(eval_mat());
    switch(p){
        case 2:{ return true; }                                         // KK
        case 3:{
            switch(m){
                case KNIGHT_VALUE:  { return true; }                    // KNK
                case BISHOP_VALUE:  { return true; }                    // KBK
                case PAWN_VALUE:                                        // KPK
                    { if(draw_KPK()) return true; else return false; }
            }
            break;
        }
        case 4:{
            switch(m){
                case 0:{
                    if(bitboard[WN] != 0){ return true; }               // KNKN
                    if(bitboard[WB] != 0){                              // KBKB
                        if(draw_KBKB()) return true; else return false;
                    }
                    break;
                }
                case (2*KNIGHT_VALUE): { return true; }                 // KNNK
                case (KNIGHT_VALUE-BISHOP_VALUE):{ return true; }       // KBKN
                case BISHOP_VALUE + PAWN_VALUE:                         // KBPK
                    { if(draw_KBPK()) return true; else return false; } // KBPK
            }
            break;
        }
    }
    return false;
}

inline int Position::KRK() const{
    Color lkp = stm;
    if(bitboard[ROOK + lkp] != 0) lkp ^= 1;

    Square lk =first_bit(bitboard[KING + lkp]);
    Bitboard bbr = bitboard[ROOK+(lkp^1)];
    if( stm == lkp
        &&((king_attack(lk) & bbr) != 0)
        && ((king_attack(first_bit(bitboard[KING + (lkp^1)])) & bbr) == 0))
        return DRAW;

    File f = file(lk);
    Rank r = rank(lk);
    int dist = std::min(std::min((FILE_H-f),int(f)),std::min((RANK_8-r),int(r)));
    int score = QUEEN_VALUE + ROOK_VALUE
                - dist * PAWN_VALUE
                - distance(first_bit(bitboard[WK]),first_bit(bitboard[BK]))
                    * (PAWN_VALUE/2.0);
    if(lkp == stm) score = -score;
    return score;
}

inline int Position::KQK() const {
    Color lkp = stm;
    if(bitboard[QUEEN + lkp] != 0) lkp ^= 1;

    Square lk =first_bit(bitboard[KING + lkp]);
    Bitboard bbq = bitboard[QUEEN+(lkp^1)];
    if(((king_attack(lk) & bbq) != 0)
        && ((king_attack(first_bit(bitboard[KING + (lkp^1)])) & bbq) == 0))
        return DRAW;

    File f = file(lk);
    Rank r = rank(lk);
    int dist = std::min(std::min((FILE_H-f),int(f)),std::min((RANK_8-r),int(r)));
    int score = QUEEN_VALUE + QUEEN_VALUE
                - dist * PAWN_VALUE
                - distance(first_bit(bitboard[WK]),first_bit(bitboard[BK]))
                    * (PAWN_VALUE/2.0);
    if(lkp == stm) score = -score;
    return score;
}

inline int Position::KBBK() const{
    Color lkp = stm, bc[2];
    int index = 0;
    if(bitboard[BISHOP + lkp] != 0) lkp ^= 1;

    Bitboard bbb = bitboard[BISHOP+(lkp^1)];
    while(bbb) bc[index++] = color(first_bit_clear(bbb));

    if(bc[0] != bc[1]){
        if(stm == lkp){
            Square lk =first_bit(bitboard[KING+lkp]);
            Bitboard att = king_attack(lk) & bitboard[BISHOP+(lkp^1)];
            if((att != 0)
                && ((king_attack(first_bit(bitboard[KING+(lkp^1)])) & att) == 0))
                return DRAW;
        }
    }
    else return DRAW;

    int score= QUEEN_VALUE + BISHOP_VALUE
               - distance(first_bit(bitboard[WK]),first_bit(bitboard[BK]))
                * PAWN_VALUE
               - corner_distance(first_bit(bitboard[KING + lkp]))
                * PAWN_VALUE/2.0;

    if(lkp == stm) score = -score;
    return score;
}

inline int Position::KBNK() const{
    Color lkp = stm;
    if(bitboard[BISHOP + lkp] != 0) lkp ^= 1;

    Square lk =first_bit(bitboard[KING+lkp]);
    Bitboard att = king_attack(lk) &
                   (bitboard[BISHOP+(lkp^1)] | bitboard[KNIGHT+(lkp^1)]);
    if(att != 0){
        if(stm == lkp){
            if((king_attack(first_bit(bitboard[KING+(lkp^1)])) & att) == 0)
                return DRAW;
        }
    }

    Square b = first_bit(bitboard[BISHOP + (lkp^1)]);
    Color bc = color(b);
    int corner_distance;
    if(bc == White) corner_distance = std::min(distance(lk,H1),distance(lk,A8));
    else corner_distance = std::min(distance(lk,A1),distance(lk,H8));
    int score = QUEEN_VALUE + KNIGHT_VALUE
                - corner_distance * PAWN_VALUE
                - distance(first_bit(bitboard[WK]),first_bit(bitboard[BK]))
                    * PAWN_VALUE / 2.0 ;
    if(lkp == stm) score = -score;
    return score;
}

inline int Position::KRRK() const{
    Color lkp = stm;
    if(bitboard[ROOK + lkp] != 0) lkp ^= 1;
    Square sq = first_bit(bitboard[KING+lkp]);
    File f = file(sq);
    Rank r = rank(sq);
    if(f >= FILE_E) f = File(FILE_H -f);
    if(r >= RANK_5) r = Rank(RANK_8 -r);
    return 2 * QUEEN_VALUE + ROOK_VALUE
            - std::min(int(f),int(r)) * PAWN_VALUE/ 2;
}

inline int Position::KQQK() const{
    Color lkp = stm;
    if(bitboard[QUEEN + lkp] != 0) lkp ^= 1;
    Square sq = first_bit(bitboard[KING+lkp]);
    Bitboard bbq = bitboard[QUEEN+(lkp^1)];
    int d1 = distance(sq,first_bit_clear(bbq));
    int d2 = distance(sq,first_bit(bbq));
    return 3 * QUEEN_VALUE
            - (d1+d2) * PAWN_VALUE/ 2;
}

inline int Position::KQRK() const{
    Color lkp = stm;
    if(bitboard[QUEEN + lkp] != 0) lkp ^= 1;
    Square sq = first_bit(bitboard[KING+lkp]);
    int d1 = distance(sq,first_bit(bitboard[QUEEN+(lkp^1)]));
    int d2 = distance(sq,first_bit(bitboard[ROOK+(lkp^1)]));
    return 3 * QUEEN_VALUE
            - PAWN_VALUE
            - (d1+d2) * PAWN_VALUE/ 2;
}

inline void Position::draw_list(Square list[8], const Color c) const{
    int pos = 0;
    Bitboard bb;
    for(Piece p = PAWN+c; p<= KING+c; p+=2){
        bb = bitboard[p];
        while(bb) list[pos++] = first_bit_clear(bb);
    }
    for(Piece p = PAWN+(c^1); p<= KING+(c^1); p+=2){
        bb = bitboard[p];
        while(bb) list[pos++] = first_bit_clear(bb);
    }
    list[pos] = NO_SQ;
    Square pawn_sq = first_bit(bitboard[PAWN+c]);
    if(file(pawn_sq) >= FILE_E){
        for(int i=0; i<pos; i++){
            list[i] = file_mirror_sq(list[i]);
        }
    }
    if(c == Black){
        for(int i=0; i<pos; i++){
            list[i] = rank_mirror_sq(list[i]);
        }
    }
}

inline bool Position::draw_KPKQ(const Color c, const Square list[8]) const{
    Square wk, wp, bk, bq, prom;
    int dist;
    wp = list[0];
    wk = list[1];
    bq = list[2];
    bk = list[3];
    if (wp == A7) {
        prom = A8;
        dist = 4;
        if(wk == B7 || wk == B8) {
            if(c == White) dist--;
        }
        else if(wk == A8 || ((wk == C7 || wk == C8) && bq != A8)) {
            if(c == Black && file(bq) != FILE_B) return false;
        } else { return false; }
        if(distance(bk,prom) > dist) return true;
    }else if(wp == C7){
        prom = C8;
        dist = 4;
        if (wk == C8) {
            dist++;
            if(c == White) dist--;
        } else if (wk == B7 || wk == B8) {
            dist--;
            if(QueenAttack[bq] & Bitboard(1)<<wk) dist++;
            if(c == White) dist--;
        }else if (wk == D7 || wk == D8){
                        if(QueenAttack[bq] & Bitboard(1)<<wk) dist++;
            if(c == White) dist--;
        } else if ((wk == A7 || wk == A8) && bq != C8) {
            if(c == Black && file(bq) != FILE_B)
                return false;
            dist--;
        } else if((wk == E7 || wk == E8) && bq != C8) {
            if(c == Black && file(bq) != FILE_D)
                return false;
        } else { return false; }
        if(distance(bk,prom) > dist) return true;
    }
    return false;
}

inline void Position::draw_KQKP(int mul[2]) const {
    Square list[8];
    if(bitboard[QUEEN+White] != 0){
        draw_list(list, Black);
        if(draw_KPKQ(stm^1,list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if(draw_KPKQ(stm,list)){
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KPKR(const Color c, const Square list[8]) const{
    Square prom;
    int dist;
    const Square wp = list[0];
    const Square wk = list[1];
    const Square br = list[2];
    const Square bk = list[3];
    const File wk_f = file(wk);
    const File wp_f = file(wp);
    const File br_f = file(br);
    const Rank wk_r = rank(wk);
    const Rank wp_r = rank(wp);
    const Rank br_r = rank(br);
    prom = pawn_promote(wp,White);
    if(distance(wk,wp) == 1){ }
    else if(distance(wk,wp) == 2 && abs(wk_r-wp_r) <= 1) {
        if(c == Black && br_f != (wk_f + wp_f) / 2) return false;
    } else {
        return false;
    }
    dist = distance(wk,prom) + distance(wp,prom);
    if (wk == prom) dist++;
    if (wk == wp+8) {
        if (wp_f == FILE_A) return false;
        dist++;
    }
    if (br_f != wp_f && br_r != RANK_8) {
        dist--;
    }
    if (c == White) dist--;
    if (distance(bk,prom) > dist) return true;
    return false;
}

inline void Position::draw_KRKP(int mul[2]) const {
    Square list[8];
    if(bitboard[ROOK+White] != 0){
        draw_list(list, Black);
        if(draw_KPKR(stm^1,list)){
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if(draw_KPKR(stm,list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KPKB(const Square list[8]) const{
    Square wp, bk, bb;
    wp = list[0];
    bb = list[2];
    bk = list[3];
    Bitboard b = Bitboard(1) << bk;
    if(bit_gt(BB_FILE[file(wp)]&((Bitboard(1)<<bb)|bb_bishop_attack(bb,b)),
              rank(wp)))
        return true;
    return false;
}

inline void Position::draw_KBKP(int mul[2]) const {
    Square list[8];
    if(bitboard[BISHOP+White] != 0){
        draw_list(list, Black);
        if (draw_KPKB(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if(draw_KPKB(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KPKN(const Square list[8]) const{
    Square wp, bn;
    wp = list[0];
    bn = list[2];
        if(bit_gt(BB_FILE[file(wp)] & ~BB_A8 & ((Bitboard(1) << bn)
        | KnightAttack[bn]),rank(wp)))
        return true;
    return false;
}

inline void Position::draw_KNKP(int mul[2]) const {
    Square list[8];
    if(bitboard[KNIGHT+White] != 0){
        draw_list(list, Black);
        if (draw_KPKN(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if (draw_KPKN(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KNPK2(const Square list[8]) const{
    Square wp, bk;
    wp = list[0];
    bk = list[3];
    if (wp == A7 && distance(bk,A8) <= 1) return true;
    return false;
}

inline void Position::draw_KNPK(int mul[2]) const {
    Square list[8];
    if(bitboard[KNIGHT+White] != 0){
        draw_list(list, Black);
        if (draw_KNPK2(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if (draw_KNPK2(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KRPKR2(const Square list[8]) const{
    Square wk, wr, wp, bk, br, prom;
    File wp_f, bk_f, br_f ;
    Rank wp_r, bk_r, br_r;
    wp = list[0];
    wr = list[1];
    wk = list[2];
    br = list[3];
    bk = list[4];
    wp_f = file(wp);
    wp_r = rank(wp);
    bk_f = file(bk);
    bk_r = rank(bk);
    br_f = file(br);
    br_r = rank(br);
    prom = pawn_promote(wp,White);
    if (bk == prom) {
        if (br_f > wp_f) return true;
    } else if (bk_f == wp_f && bk_r > wp_r) {
        return true;
    } else if (wr == prom && wp_r == RANK_7
               && (bk == G7 || bk == H7) && br_f == wp_f) {
        if (br_r <= RANK_3) {
            if (distance(wk,wp) > 1) return true;
        } else {
            if (distance(wk,wp) > 2) return true;
        }
    }
    return false;
}

inline void Position::draw_KRPKR(int mul[2]) const {
    Square list[8];
    if(bitboard[PAWN+White] != 0){
        draw_list(list, Black);
        if (draw_KRPKR2(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if (draw_KRPKR2(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KBPKB2(const Square list[8]) const{
    const Square wp = list[0];
    const Square wb = list[1];
    const Square bb = list[3];
    const Square bk = list[4];
    if(color(wb) == color(bb)) return false;
    const Bitboard b = Bitboard(1) << bk;
    if(bit_gt(BB_FILE[file(wp)]&((Bitboard(1)<<bb)|bb_bishop_attack(bb,b)),
        rank(wp)))
        return true;
    return false;
}

inline void Position::draw_KBPKB(int mul[2]) const {
    Square list[8];
    if(bitboard[PAWN+White] != 0){
        draw_list(list, Black);
        if(draw_KBPKB2(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
    else{
        draw_list(list, White);
        if (draw_KBPKB2(list)) {
            if(mul[White] > 1) mul[White] = 1;
            if(mul[Black] > 1) mul[Black] = 1;
        }
    }
}

inline bool Position::draw_KPK2(const Square wp, const Square wk,
                               const Square bk, const Color c) const{
    int wp_file, wp_rank;
    int wk_file;
    int bk_file, bk_rank;

    wp_file = file(wp);
    wp_rank = rank(wp);

    wk_file = file(wk);

    bk_file = rank(bk);
    bk_rank = rank(bk);

    if (bk == wp+8) {
       if (wp_rank <= RANK_6) {
          return true;
       } else {
          if (c == White) {
             if (wk == wp-7 || wk == wp-9) return true;
          } else {
             if (wk != wp-7 && wk != wp-9) return true;
          }
       }
    } else if (bk == wp+16) {
       if (wp_rank <= RANK_5) {
          return true;
       } else {
          if (c == White) {
             if (wk != wp-1 && wk != wp+1) return true;
          } else {
             return true;
          }
       }
    } else if (wk == wp-1 || wk == wp+1) {
       if (bk == wk+16 && c == White) {
          return true;
       }
    } else if (wk == wp+7 || wk == wp+8 || wk == wp+9) {
       if (wp_rank <= RANK_4) {
          if (bk == wk+16 && c == White) {
             return true;
          }
       }
    }

    if (wp_file == FILE_A) {
       if (distance(bk,A8) <= 1) return true;
       if (wk_file == FILE_A) {
          if (wp_rank == RANK_2) wp_rank++; // HACK
          if (bk_file == FILE_C && bk_rank > wp_rank) return true;
       }
    }
    return false;
}

inline bool Position::draw_KPK() const{
    Square wp, wk, bk;

    if(bitboard[PAWN+White]){
        wp = first_bit(bitboard[PAWN+White]);
        wk = first_bit(bitboard[KING+White]);
        bk = first_bit(bitboard[KING+Black]);

        if (file(wp) >= FILE_E) {
            wp = file_mirror_sq(wp);
            wk = file_mirror_sq(wk);
            bk = file_mirror_sq(bk);
        }

        if (draw_KPK2(wp,wk,bk,stm)) return true;
    }
    else{
        wp = rank_mirror_sq(first_bit(bitboard[PAWN+Black]));
        wk = rank_mirror_sq(first_bit(bitboard[KING+Black]));
        bk = rank_mirror_sq(first_bit(bitboard[KING+White]));

        if (file(wp) >= FILE_E) {
            wp = file_mirror_sq(wp);
            wk = file_mirror_sq(wk);
            bk = file_mirror_sq(bk);
        }

        if (draw_KPK2(wp,wk,bk,stm^1)) return true;
    }

    return false;
}

inline bool Position::draw_KBKB() const{
    const Bitboard corners = BB_A1 | BB_A8 | BB_H1 | BB_H8;
    const Bitboard corners2 = BB_A2 | BB_B1 | BB_G1 | BB_H2 |
                              BB_A7 | BB_B8 | BB_G8 | BB_H7;
    const Square wb = first_bit(bitboard[BISHOP+White]);
    const Square bb = first_bit(bitboard[BISHOP+Black]);
    const Square wk = first_bit(bitboard[KING+White]);
    const Square bk = first_bit(bitboard[KING+Black]);
    if(color(wb) != color(bb) && distance(wk,bk) == 2){
        for(Color c=White; c<= Black; c++){
            if((bitboard[KING+c] & corners)
                && (bitboard[BISHOP+c] & corners2
                    & king_attack(first_bit(bitboard[KING+c])))){
                return false;
            }
        }
    }
    return true;
}

inline bool Position::draw_KBPK() const{
    Square wp, wb, bk;
    if(bitboard[PAWN+White]){
        wp = first_bit(bitboard[PAWN+White]);
        wb = first_bit(bitboard[BISHOP+White]);
        bk = first_bit(bitboard[KING+Black]);
    }
    else{
        wp = rank_mirror_sq(first_bit(bitboard[PAWN+Black]));
        wb = rank_mirror_sq(first_bit(bitboard[BISHOP+Black]));
        bk = rank_mirror_sq(first_bit(bitboard[KING+White]));
    }

    if (file(wp) >= FILE_E) {
        wp = file_mirror_sq(wp);
        wb = file_mirror_sq(wb);
        bk = file_mirror_sq(bk);
    }

    if (file(wp) == FILE_A && distance(bk,A8) <= 1 && color(wb) != color(A8))
       return true;
    return false;
}
