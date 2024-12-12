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
 ***************************************************************************
 *   This code based on Toga II and Stochfish code.                        *
 ***************************************************************************/

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cstring>

#include "position.h"
#include "thread.h"

const int Position::TrappedBishop = 100;
const int Position::BlockedBishop = 50;
const int Position::BlockedRook = 50;
const int Position::BlockedCenterPawn = 10;

const int Position::KnightUnit = 4;
const int Position::BishopUnit = 6;
const int Position::RookUnit = 7;

const int Position::KnightMob[2] = {4, 4};
const int Position::BishopMob[2] = {5, 5};
const int Position::RookMob[2] = {2, 4};
const int Position::QueenMob[2] = {1, 2};
const int Position::KingMob[2] = {0, 0};
const int Position::RookSemiOpenFile[2] = {10, 10};
const int Position::RookOpenFile[2] = {20, 20} ;
const int Position::RookSemiKingFileOpening = 10;
const int Position::RookKingFileOpening = 20;
const int Position::Rook7th[2] = {20, 40};
const int Position::Queen7th[2] = {10, 20};
const int Position::StormOpening = 10;
const int Position::KingAttackOpening = 20;
const int Position::Doubled[2] = {10, 20};
const int Position::Isolated[2] = {10, 20};
const int Position::IsolatedOpenOpening = 20;
const int Position::Backward[2] = {8, 10};
const int Position::BackwardOpenOpening = 16;
const int Position::CandidateMin[2] = {5, 10};
const int Position::CandidateMax[2] = {55, 110};
const int Position::AttackerDistance = 5;
const int Position::DefenderDistance = 20;
const int Position::ShelterOpening = 256;
const int Position::RedundantQueenPenalty = 320;
const int Position::RedundantRookPenalty  = 554;

const int Position::PiecePhase[12] = {20, 20, 0, 0, 1, 1, 1, 1, 2, 2, 4, 4};
const int Position::TotalPhase = 24;

const int Position::KingAttackUnit[14] = {
    0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 4, 4, 0 ,0
};
const int Position::KingAttackWeight[16] = {
    0, 0, 128, 192, 224, 240, 248, 252, 254, 255, 256, 256 ,256, 256, 256, 256
};

const int Position::Bonus[8] = {0,0,0,26,77,154,256,0};

const int Position::KnightOutpost[64] = {
    0, 0,  0,  0,  0,  0, 0, 0,
    0, 0,  0,  0,  0,  0, 0, 0,
    0, 0,  2,  4,  4,  2, 0, 0,
    0, 2,  8, 12, 12,  8, 2, 0,
    0, 4, 13, 17, 17, 13, 4, 0,
    0, 2,  8,  8,  8,  8, 2, 0,
    0, 0,  0,  0,  0,  0, 0, 0,
    0, 0,  0,  0,  0,  0, 0, 0
};

const int Position::BishopOutpost[64] = {
    0, 0,  0,  0,  0,  0, 0, 0,
    0, 0,  0,  0,  0,  0, 0, 0,
    0, 0,  2,  2,  2,  2, 0, 0,
    0, 2,  5,  5,  5,  5, 2, 0,
    0, 5, 10, 10, 10, 10, 5, 0,
    0, 2,  4,  4,  4,  4, 2, 0,
    0, 0,  0,  0,  0,  0, 0, 0,
    0, 0,  0,  0,  0,  0, 0, 0
};

const int Position::SSpaceWeight[16] = {
    0, 2, 5, 8, 12, 15, 20, 20, 20, 20, 20, 20 ,20, 20, 20, 20,
};

const int Position::ExchangePenalty[2] = {30, 30};
const int Position::BishopPair[2] = {50,70};

const int Position::DrawNodeFlag    = 1 << 0;
const int Position::DrawBishopFlag    = 1 << 1;

const int Position::MatRookPawnFlag    = 1 << 0;
const int Position::MatBishopFlag    = 1 << 1;
const int Position::MatKnightFlag    = 1 << 2;
const int Position::MatKingFlag        = 1 << 3;

#define TXX {0,0}
#define TPN {28,35}
#define TPB {28,35}
#define TPR {38,49}
#define TPQ {43,59}
#define TNP {3,19}
#define TNB {12,24}
#define TNR {20,50}
#define TNQ {20,50}
#define TBP {3,19}
#define TBN {12,24}
#define TBR {20,50}
#define TBQ {20,50}
#define TRP {0,14}
#define TRN {7,24}
#define TRB {7,24}
#define TRQ {12,24}
#define TQP {7,19}
#define TQN {7,19}
#define TQB {7,19}
#define TQR {7,19}

const int Position::ThreatBonus[12][12][2] = {
// W    B    WP   BP   WN   BN   WB   BB   WR   BR   WQ   BQ
 {TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX},  // W
 {TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX, TXX},  // B
 {TXX, TXX, TXX, TXX, TXX, TPN, TXX, TPB, TXX, TPR, TXX, TPQ},  // WP
 {TXX, TXX, TXX, TXX, TPN, TXX, TPB, TXX, TPR, TXX, TPQ, TXX},  // BP
 {TXX, TXX, TXX, TNP, TXX, TXX, TXX, TNB, TXX, TNR, TXX, TNQ},  // WN
 {TXX, TXX, TNP, TXX, TXX, TXX, TNB, TXX, TNR, TXX, TNQ, TXX},  // BN
 {TXX, TXX, TXX, TBP, TXX, TBN, TXX, TXX, TXX, TBR, TXX, TBQ},  // WB
 {TXX, TXX, TBP, TXX, TBN, TXX, TXX, TXX, TBR, TXX, TBQ, TXX},  // BB
 {TXX, TXX, TXX, TRP, TXX, TRN, TXX, TRB, TXX, TXX, TXX, TRQ},  // WR
 {TXX, TXX, TRP, TXX, TRN, TXX, TRB, TXX, TXX, TXX, TRQ, TXX},  // BR
 {TXX, TXX, TXX, TQP, TXX, TQN, TXX, TQB, TXX, TQR, TXX, TXX},  // WQ
 {TXX, TXX, TQP, TXX, TQN, TXX, TQB, TXX, TQR, TXX, TXX, TXX},  // BQ
};

const int Position::LinearCoefficients[12] = {
    1617, 1617, -162, -162, -1172, -1172, -190, -190, 105, 105, 26, 26
};

const int Position::QuadraticCoefficientsSameColor[12][12] = {
    {  7,   7,   7,   7,  7,  7,   7,   7,  7,  7,   7,   7},
    {  7,   7,   7,   7,  7,  7,   7,   7,  7,  7,   7,   7},
    { 39,  39,   2,   2,  7,  7,   7,   7,  7,  7,   7,   7},
    { 39,  39,   2,   2,  7,  7,   7,   7,  7,  7,   7,   7},
    { 35,  35, 271, 271, -4, -4,   7,   7,  7,  7,   7,   7},
    { 35,  35, 271, 271, -4, -4,   7,   7,  7,  7,   7,   7},
    {  7,   7,  25,  45,  4,  4,   7,   7,  7,  7,   7,   7},
    {  7,   7,  25,  45,  4,  4,   7,   7,  7,  7,   7,   7},
    {-27, -27,  -2,  -2, 46, 46, 100, 100, 56, 56,   7,   7},
    {-27, -27,  -2,  -2, 46, 46, 100, 100, 56, 56,   7,   7},
    { 58,  58,  29,  29, 83, 83, 148, 148, -3, -3, -25, -25},
    { 58,  58,  29,  29, 83, 83, 148, 148, -3, -3, -25, -25}
};

const int Position::QuadraticCoefficientsOppositeColor[12][12] = {
    { 41,  41,  41,  41, 41, 41,  41,  41,  41,  41, 41, 41},
    { 41,  41,  41,  41, 41, 41,  41,  41,  41,  41, 41, 41},
    { 37,  37,  41,  41, 41, 41,  41,  41,  41,  41, 41, 41},
    { 37,  37,  41,  41, 41, 41,  41,  41,  41,  41, 41, 41},
    { 10,  10,  62,  62, 41, 41,  41,  41,  41,  41, 41, 41},
    { 10,  10,  62,  62, 41, 41,  41,  41,  41,  41, 41, 41},
    { 57,  57,  64,  64, 39, 39,  41,  41,  41,  41, 41, 41},
    { 57,  57,  64,  64, 39, 39,  41,  41,  41,  41, 41, 41},
    { 50,  50,  40,  40, 23, 23, -22, -22,  41,  41, 41, 41},
    { 50,  50,  40,  40, 23, 23, -22, -22,  41,  41, 41, 41},
    {106, 106, 101, 101,  3,  3, 151, 151, 171, 171, 41, 41},
    {106, 106, 101, 101,  3,  3, 151, 151, 171, 171, 41, 41}
};

template <bool print_info>
int Position::eval(Thread & thread, int & margin) const{
    int eval;
    EvalInfo info;
    margin = 0;

    int piece_num = thread.pos.num_pieces();
    info.mul[White] = info.mul[Black] = 16;
    if(piece_num <= 5 && eg_eval(eval,info)) return eval;

    // Tempo + PVT
    if(stm == White){
        info.eval[Opening] = 20 + current->pvt[Opening];
        info.eval[Endgame] = 10 + current->pvt[Endgame];
    }
    else{
        info.eval[Opening] = -20 + current->pvt[Opening];
        info.eval[Endgame] = -10 + current->pvt[Endgame];
    }

    MaterialHash * mh = &thread.material_hash[current->mat_key%thread.mh_size];
    if(print_info || mh->key != current->mat_key) eval_mat<print_info>(mh);
    info.eval[Opening] += mh->eval[Opening];
    info.eval[Endgame] += mh->eval[Endgame];

    if(print_info){
        std::cout<<std::setw(19)<<"Mat/PVT/Tem/Imb |"<<std::setw(8);
        std::cout<<info.eval[Opening]<<" |"<<std::setw(8)<<info.eval[Endgame];
        std::cout<<" |"<<std::endl;
    }


    eval_draw(info, mh);

    Bitboard pawn = bitboard[PAWN+White];
    info.attacked_by[PAWN+White] = ((pawn << 7 & BB_NOT_FILE_H)
                                    | (pawn << 9 & BB_NOT_FILE_A));

    pawn = bitboard[PAWN+Black];
    info.attacked_by[PAWN+Black] = ((pawn >> 7 & BB_NOT_FILE_A)
                                    | (pawn >> 9 & BB_NOT_FILE_H));

    PawnHash * ph = &thread.pawn_hash[current->pawn_key % thread.ph_size];
    if(print_info || ph->key != current->pawn_key){
        ph->eval[Opening] = ph->eval[Endgame] = 0;
        eval_pawns<White,print_info>(ph);
        eval_pawns<Black,print_info>(ph);
    }
    info.eval[Opening] += ph->eval[Opening];
    info.eval[Endgame] += ph->eval[Endgame];

    eval_patterns<White,print_info>(info);
    eval_patterns<Black,print_info>(info);
    eval_pieces<White,print_info>(info,mh);
    eval_pieces<Black,print_info>(info,mh);
    eval_king<White,print_info>(info,mh,margin);
    eval_king<Black,print_info>(info,mh,margin);
    eval_threats<White,print_info>(info);
    eval_threats<Black,print_info>(info);
    eval_passer<White,print_info>(info,ph);
    eval_passer<Black,print_info>(info,ph);

    if(print_info){
        std::cout<<"======================================="<<std::endl;
        std::cout<<std::setw(19)<<"Eval |"<<std::setw(8)<<info.eval[Opening];
        std::cout<<" |"<<std::setw(8)<<info.eval[Endgame]<<" |"<<std::endl;
    }

    eval = ((info.eval[Opening] * (256 - mh->phase)) +
            (info.eval[Endgame] * mh->phase)) / 256;

    if (eval > 0) eval = (eval * info.mul[White]) / 16;
    else if (eval < 0) eval = (eval * info.mul[Black]) / 16;

    if(print_info){
        std::cout<<"---------------------------------------"<<std::endl;
        std::cout<<std::setw(19)<<" |"<<std::setw(8)<<"White";
        std::cout<<" |"<<std::setw(8)<<"Black"<<" |"<<std::endl;
        std::cout<<"---------------------------------------"<<std::endl;
        std::cout<<std::setw(19)<<"Mul |"<<std::setw(8)<<info.mul[White]/16;
        std::cout<<" |"<<std::setw(8)<<info.mul[Black]/16<<" |"<<std::endl;
        std::cout<<"---------------------------------------"<<std::endl;
        std::cout<<std::setw(19)<<" |"<<std::setw(8)<<"Eval";
        std::cout<<" |"<<std::setw(8)<<"Margine"<<" |"<<std::endl;
        std::cout<<"======================================="<<std::endl;
        std::cout<<" Eval (phase="<<int((mh->phase/256.0)*100)<<") |";
        std::cout<<std::setw(8)<<eval<<" |"<<std::setw(8)<<margin<<" |";
        std::cout<<std::endl;    }

    if(stm == Black) eval = -eval;

    return eval;
}

int Position::eval_mat() const{
    int r = (num[WP]-num[BP])*PAWN_VALUE +
            (num[WN]-num[BN])*KNIGHT_VALUE +
            (num[WB]-num[BB])*BISHOP_VALUE +
            (num[WR]-num[BR])*ROOK_VALUE +
            (num[WQ]-num[BQ])*QUEEN_VALUE;
    if(stm == Black) r = -r;
    return r;
}

template <bool print_info>
void Position::eval_mat(MaterialHash* mh) const{
    int flags, cflags[2], mul[2];

    const int wp = num[WP];
    const int wn = num[WN];
    const int wb = num[WB];
    const int wr = num[WR];
    const int wq = num[WQ];

    const int bp = num[BP];
    const int bn = num[BN];
    const int bb = num[BB];
    const int br = num[BR];
    const int bq = num[BQ];

    const int wt = num[W]-1;
    const int bt = num[B]-1;

    flags = cflags[White] = cflags[Black] = 0;

    if (wq+wr+wn == 0 && bq+br+bn == 0) {
        if (wb == 1 && bb == 1) {
            if (wp-bp >= -2 && wp-bp <= +2) {
                flags |= DrawBishopFlag;
            }
        }
    }

    mul[White] = mul[Black] = 16;

    if (wp == 0) {
        int w_maj = wq * 2 + wr;
        int w_min = wb + wn;
        int w_tot = w_maj * 2 + w_min;

        int b_maj = bq * 2 + br;
        int b_min = bb + bn;
        int b_tot = b_maj * 2 + b_min;

        if (w_tot == 1) {
            mul[White] = 0;
        } else if (w_tot == 2 && wn == 2) {
            if (b_tot != 0 || bp == 0) {
                mul[White] = 0;
            } else {
                mul[White] = 1;
            }
        } else if (w_tot == 2 && wb == 2 && b_tot == 1 && bn == 1) {
            mul[White] = 8;
        } else if (w_tot-b_tot <= 1 && w_maj <= 2) {
            mul[White] = 2;
        }
    } else if (wp == 1) {
        int w_maj = wq * 2 + wr;
        int w_min = wb + wn;
        int w_tot = w_maj * 2 + w_min;

        int b_maj = bq * 2 + br;
        int b_min = bb + bn;
        int b_tot = b_maj * 2 + b_min;

        if (b_min != 0) {
            b_min--;
            b_tot--;

            if (w_tot == 1) {
                mul[White] = 4;
            } else if (w_tot == 2 && wn == 2) {
                mul[White] = 4;
            } else if (w_tot-b_tot <= 1 && w_maj <= 2) {
                mul[White] = 8;
            }
        } else if (br != 0) {
            b_maj--;
            b_tot -= 2;
            if (w_tot == 1) {
                mul[White] = 4;
            } else if (w_tot == 2 && wn == 2) {
                mul[White] = 4;
            } else if (w_tot-b_tot <= 1 && w_maj <= 2) {
                mul[White] = 8;
            }
        }
    }

    if (bp == 0) {
        int w_maj = wq * 2 + wr;
        int w_min = wb + wn;
        int w_tot = w_maj * 2 + w_min;

        int b_maj = bq * 2 + br;
        int b_min = bb + bn;
        int b_tot = b_maj * 2 + b_min;

        if (b_tot == 1) {
            mul[Black] = 0;
        } else if (b_tot == 2 && bn == 2) {
            if (w_tot != 0 || wp == 0) {
                mul[Black] = 0;
            } else {
                mul[Black] = 1;
            }
        } else if (b_tot == 2 && bb == 2 && w_tot == 1 && wn == 1) {
            mul[Black] = 8;
        } else if (b_tot-w_tot <= 1 && b_maj <= 2) {
            mul[Black] = 2;
        }
    } else if (bp == 1) {
        int w_maj = wq * 2 + wr;
        int w_min = wb + wn;
        int w_tot = w_maj * 2 + w_min;

        int b_maj = bq * 2 + br;
        int b_min = bb + bn;
        int b_tot = b_maj * 2 + b_min;

        if (w_min != 0) {
            w_min--;
            w_tot--;
            if (b_tot == 1) {
                mul[Black] = 4;
            } else if (b_tot == 2 && bn == 2) {
                mul[Black] = 4;
            } else if (b_tot-w_tot <= 1 && b_maj <= 2) {
                mul[Black] = 8; // 1/2
            }
        } else if (wr != 0){
            w_maj--;
            w_tot -= 2;
            if (b_tot == 1) {
                mul[Black] = 4;
            } else if (b_tot == 2 && bn == 2) {
                mul[Black] = 4;
            } else if (b_tot-w_tot <= 1 && b_maj <= 2) {
                mul[Black] = 8;
            }
        }
    }

    if (wt == wb+wp && wp >= 1)
        cflags[White] |= MatRookPawnFlag;
    if (wt == wb+wp && wb <= 1 && wp >= 1 && bt > bp)
        cflags[White] |= MatBishopFlag;
    if (wt == 2 && wn == 1 && wp == 1 && bt > bp)
        cflags[White] |= MatKnightFlag;

    if (bt == bb+bp && bp >= 1)
        cflags[Black] |= MatRookPawnFlag;
    if (bt == bb+bp && bb <= 1 && bp >= 1 && wt > wp)
        cflags[Black] |= MatBishopFlag;
    if (bt == 2 && bn == 1 && bp == 1 && wt > wp)
        cflags[Black] |= MatKnightFlag;

    if (bq >= 1 && bq+br+bb+bn >= 2) cflags[White] |= MatKingFlag;
    if (wq >= 1 && wq+wr+wb+wn >= 2) cflags[Black] |= MatKingFlag;

    int phase = TotalPhase;

    phase -= wp * PiecePhase[PAWN];
    phase -= wn * PiecePhase[KNIGHT];
    phase -= wb * PiecePhase[BISHOP];
    phase -= wr * PiecePhase[ROOK];
    phase -= wq * PiecePhase[QUEEN];

    phase -= bp * PiecePhase[PAWN];
    phase -= bn * PiecePhase[KNIGHT];
    phase -= bb * PiecePhase[BISHOP];
    phase -= br * PiecePhase[ROOK];
    phase -= bq * PiecePhase[QUEEN];

    if(phase < 0) phase = 0;

    phase = (phase * 256 + (TotalPhase / 2)) / TotalPhase;

    int opening = 0, endgame = 0;

    const int owf = wn * MAT[KNIGHT][Opening] + wb * MAT[BISHOP][Opening]
                    + wr * MAT[ROOK][Opening] + wq * MAT[QUEEN][Opening];
    opening += wp * MAT[PAWN][Opening];
    opening += owf;

    const int obf = bn * MAT[KNIGHT][Opening] + bb * MAT[BISHOP][Opening]
                    + br * MAT[ROOK][Opening] + bq * MAT[QUEEN][Opening];
    opening -= bp * MAT[PAWN][Opening];
    opening -= obf;

    const int ewf = wn * MAT[KNIGHT][Endgame] + wb * MAT[BISHOP][Endgame]
                    + wr * MAT[ROOK][Endgame] + wq * MAT[QUEEN][Endgame];
    endgame += wp * MAT[PAWN][Endgame];
    endgame += ewf;

    const int ebf = bn * MAT[KNIGHT][Endgame] + bb * MAT[BISHOP][Endgame]
                    + br * MAT[ROOK][Endgame] + bq * MAT[QUEEN][Endgame];
    endgame -= bp * MAT[PAWN][Endgame];
    endgame -= ebf;

/*    if (owf - obf > 2 * (BISHOP_VALUE - KNIGHT_VALUE)){
        opening += ExchangePenalty[Opening] + wp;
        endgame += ExchangePenalty[Endgame] + wp;
        if(wp >= bp){
            opening += ExchangePenalty[Opening] * (1 + wp - bp) + wp;
            endgame += ExchangePenalty[Endgame] * (1 + wp - bp) + wp;
        }
    }
    else if (obf - owf > 2 * (BISHOP_VALUE - KNIGHT_VALUE)){
        opening -= ExchangePenalty[Opening] + bp;
        endgame -= ExchangePenalty[Endgame] + bp;
        if(bp >= wp){
            opening -= ExchangePenalty[Opening] * (1 + bp - wp) + bp;
            endgame -= ExchangePenalty[Endgame] * (1 + bp - wp) + bp;
        }
    }

    if (wb >= 2) {
        opening += BishopPair[Opening];
        endgame += BishopPair[Endgame];
    }

    if (bb >= 2) {
        opening -= BishopPair[Opening];
        endgame -= BishopPair[Endgame];
    }*/

    int num_pieces[14];
    memcpy(num_pieces,num,14*sizeof(int));
    num_pieces[White] = num[BISHOP+White] > 1;
    num_pieces[Black] = num[BISHOP+Black] > 1;
    int imbalance = (eval_imbalance<White>(num_pieces)
                     -eval_imbalance<Black>(num_pieces))/32;
    opening += imbalance;
    endgame += imbalance;
    if(print_info){
        std::cout<<std::setw(17)<<"* Imbalance "<<" |";
        std::cout<<std::setw(8)<<imbalance<<" |";
        std::cout<<std::setw(8)<<imbalance<<" |"<<std::endl;
        std::cout<<"---------------------------------------"<<std::endl;
    }

    mh->key = current->mat_key;
    mh->phase = phase;
    mh->eval[Opening] = opening;
    mh->eval[Endgame] = endgame;
    mh->mul[White] = mul[White];
    mh->mul[Black] = mul[Black];
    mh->cflags[White] = cflags[White];
    mh->cflags[Black] = cflags[Black];
    mh->flags = flags;
}

template <Color c>
int Position::eval_imbalance(const int num_pieces[14]) const{
    int value = 0, v, pc;
    Piece p1, p2;
    Color opp;
    if(c== White) opp = 1;
    else opp = -1;

    if (num_pieces[ROOK+c] > 0)
        value -=  RedundantRookPenalty * (num_pieces[ROOK+c] - 1)
                + RedundantQueenPenalty * num_pieces[QUEEN+c];


    for (p1 = c; p1 <= QUEEN+c; p1+=2)
    {
        pc = num_pieces[p1];

        if (!pc) continue;

        v = LinearCoefficients[p1];

        for (p2 = c; p2 <= p1; p2+=2){
            v +=  QuadraticCoefficientsSameColor[p1][p2] * num_pieces[p2]
                  + QuadraticCoefficientsOppositeColor[p1][p2] * num_pieces[p2+opp];
        }

        value += pc * v;
    }
    return value;
}

void Position::eval_draw(EvalInfo & info, const MaterialHash* mh) const{
    Color c, opp;
    Square pSq, kSq, sf_p_sq, prom_sq;
    File f;
    Rank r;
    Bitboard pawns;

    if(num[White] >= 10 && num[Black] >= 10) return;

    const int cflags[2] = { mh->cflags[White], mh->cflags[Black] };
    int mul[2] = { mh->mul[White], mh->mul[Black] };

    for(c = White; c <= Black; c++) {
        opp = c^1;

        pawns = bitboard[PAWN+c];
        pSq = first_bit(pawns);

        sf_p_sq = NO_SQ;
        if(pawns != 0 && (~(BB_FILE[file(pSq)]) & pawns) == 0){
            if(c == White) sf_p_sq = first_bit(pawns);
            else sf_p_sq = last_bit(pawns);
            f = file(sf_p_sq);
            r = rank(sf_p_sq);
            if((c == White
                && !bit_gt(bb_file(bitboard[PAWN+opp],f-1),r)
                && !bit_gt(bb_file(bitboard[PAWN+opp],f+1),r))
                ||    (c == Black
                     && !bit_le(bb_file(bitboard[PAWN+opp],f-1),r)
                     && !bit_le(bb_file(bitboard[PAWN+opp],f+1),r)) ){
                if(c == White) sf_p_sq = last_bit(pawns);
                else sf_p_sq = first_bit(pawns);
            }
            else sf_p_sq = NO_SQ;
        }
        if(sf_p_sq != NO_SQ && (cflags[c] & MatRookPawnFlag) != 0) {
            f = file(sf_p_sq);
            r = rank(sf_p_sq);
            if(f == FILE_A || f == FILE_H){
                kSq = first_bit(bitboard[KING + opp]);
                prom_sq = pawn_promote(sf_p_sq,c);
                if(distance(kSq, prom_sq) <= 1
                   && !bishop_can_attack(prom_sq,c)){
                    mul[c] = 0;
                }
            }
        }
        if(sf_p_sq != NO_SQ && (cflags[c] & MatBishopFlag) != 0) {
            kSq = first_bit(bitboard[KING + opp]);
            if(file(kSq) == file(sf_p_sq)
                && ((c == White && rank(kSq) > rank(sf_p_sq))
                    || (c == Black && rank(kSq) < rank(sf_p_sq)))
                && !bishop_can_attack(kSq,c)){
                mul[c] = 1;
            }
        }
        if ((cflags[c] & MatKnightFlag) != 0) {
            kSq = first_bit(bitboard[KING + opp]);
            if(file(kSq) == file(pSq)
                && ((c == White && rank(kSq) > rank(pSq)
                     && rank(pSq) <= RANK_6)
                    || (c == Black && rank(kSq) < rank(pSq)
                        && rank(pSq) >= RANK_3))){
                if(mul[c] > 1) mul[c] = 1;
            }
        }
    }

    if ((mh->flags & DrawBishopFlag) != 0) {
        Square wb = first_bit(bitboard[WB]);
        Square bb = first_bit(bitboard[BB]);
        if (color(wb) != color(bb)) {
            if (mul[White] == 16) mul[White] = 8;
            if (mul[Black] == 16) mul[Black] = 8;
        }
    }
    info.mul[White] = std::min(mul[White],info.mul[White]);
    info.mul[Black] = std::min(mul[Black],info.mul[Black]);
}

template <Color c, bool print_info>
void Position::eval_pieces(EvalInfo & info, const MaterialHash* mh) const{
    int att_num,att_value,penalty,opening,endgame,mob,outpost,obonus,ebonus;
    const Color opp = c^1;
    File f;
    Rank r;
    Bitboard bb, sq_attack, sq_attack2;

    opening = endgame = 0;
    att_num = att_value = 0;
    info.att_num[opp] = info.att_total[opp] = 0;
    Square sq = first_bit(bitboard[WK+opp]);
    File kF = file(sq);
    Rank kR = rank(sq);
    Bitboard king_safety2 = KingAttack2[sq];
    Bitboard king_safety = KingAttack[sq];
    Bitboard mob_attack = ~(bitboard[c] | info.attacked_by[PAWN+opp]);
    info.attacked_by[c] = info.attacked_by[PAWN+c];
    if(print_info)
        std::cout<<"---------------------------------------"<<std::endl;

    for(Piece p=KNIGHT+c; p<=QUEEN+c; p+=2){
        bb = bitboard[p];
        info.attacked_by[p] = 0;
        while(bb){
            sq = first_bit_clear(bb);
            sq_attack = (this->*attack[p])(sq);
            info.attacked_by[c] |= sq_attack;
            info.attacked_by[p] |= sq_attack;
            if(info.attacked_by[PAWN+opp] & (Bitboard(1) << sq)){
                opening -= ThreatBonus[PAWN+opp][p][Opening];
                endgame -= ThreatBonus[PAWN+opp][p][Endgame];
            }
            switch (p){
                case KNIGHT+c:{
                    if(sq_attack & king_safety){
                        info.att_num[opp] ++;
                        info.att_total[opp] += KingAttackUnit[KNIGHT];
                    }
                    mob = - KnightUnit + bit_count_15(sq_attack&mob_attack);
                    outpost = KnightOutpost[relative_sq<c>(sq)];
                    if(outpost && !(info.attacked_by[PAWN+opp] & (Bitboard(1)<<sq))){
                        if(info.attacked_by[PAWN+c] & (Bitboard(1)<<sq)){
                            if( bitboard[KNIGHT+opp] == 0 &&
                                !(BBColorSquare[color(sq)] & bitboard[BISHOP+opp])){
                                outpost += outpost + outpost / 2;
                            }
                            else outpost += outpost/2;
                        }
                    }
                    if(print_info){
                        std::cout<<std::setw(15)<<"* Knight ["<<piece_to_char(c)<<"] |";
                        std::cout<<std::setw(8)<<relative_value<c>(mob * KnightMob[Opening] + outpost)<<" |";
                        std::cout<<std::setw(8)<<relative_value<c>(mob * KnightMob[Endgame])<<" |"<<std::endl;
                    }
                    opening += mob * KnightMob[Opening] + outpost;
                    endgame += mob * KnightMob[Endgame];
                    break;
                }
                case BISHOP+c:{
                    sq_attack2 = sq_attack & king_safety2;
                    if(sq_attack2){
                        if(sq_attack & king_safety || bb_bishop_attack(
                                sq,occupied ^ (occupied & sq_attack2))
                               & king_safety){
                            info.att_num[opp] ++;
                            info.att_total[opp] += KingAttackUnit[BISHOP];
                        }
                    }
                    else if(bb_bishop_attack(sq,occupied^bitboard[QUEEN+c])
                              & king_safety){
                        info.att_num[opp] ++;
                        info.att_total[opp] += KingAttackUnit[BISHOP];
                    }
                    mob = - BishopUnit + bit_count_15(sq_attack&mob_attack);
                    outpost = BishopOutpost[relative_sq<c>(sq)];
                    if(outpost
                       && !(info.attacked_by[PAWN+opp] & (Bitboard(1)<<sq))){
                        if(info.attacked_by[PAWN+c] & (Bitboard(1)<<sq)){
                            if( bitboard[KNIGHT+opp] == 0 &&
                                !(BBColorSquare[color(sq)] & bitboard[BISHOP+opp])){
                                outpost += outpost + outpost / 2;
                            }
                            else outpost += outpost / 2;
                        }
                    }
                    if(print_info){
                        std::cout<<std::setw(15)<<"* Bishop ["<<piece_to_char(c)<<"] |";
                        std::cout<<std::setw(8)<<relative_value<c>(mob * BishopMob[Opening] + outpost)<<" |";
                        std::cout<<std::setw(8)<<relative_value<c>(mob * BishopMob[Endgame])<<" |"<<std::endl;
                    }
                    opening += mob * BishopMob[Opening] + outpost;
                    endgame += mob * BishopMob[Endgame];
                    break;
                }
                case ROOK+c:{
                    sq_attack2 = sq_attack & king_safety2;
                    if(sq_attack2){
                        if(sq_attack & king_safety || bb_rook_attack(
                                sq,occupied ^ (occupied & sq_attack2))
                                & king_safety){
                            info.att_num[opp] ++;
                            info.att_total[opp] += KingAttackUnit[ROOK];
                        }
                    }
                    else if(bb_rook_attack(sq,occupied^(bitboard[ROOK+c]
                                           |bitboard[QUEEN+c])) & king_safety){
                        info.att_num[opp] ++;
                        info.att_total[opp] += KingAttackUnit[BISHOP];
                    }
                    mob = - RookUnit + bit_count_15(sq_attack & mob_attack);
                    obonus = -RookOpenFile[Opening] / 2;
                    ebonus = -RookOpenFile[Endgame] / 2;
                    f = file(sq);
                    r = rank(relative_sq<c>(sq));
                    sq_attack = BB_FILE[f];
                    if((sq_attack & bitboard[PAWN+c]) == 0){
                        obonus += RookSemiOpenFile[Opening];
                        ebonus += RookSemiOpenFile[Endgame];
                        if((sq_attack & bitboard[PAWN+opp]) == 0){
                            obonus += RookOpenFile[Opening]
                                           - RookSemiOpenFile[Opening];
                            ebonus += RookOpenFile[Endgame]
                                           - RookSemiOpenFile[Endgame];
                        }
                        if(mh->cflags[opp] & MatKingFlag){
                            mob = abs(f - kF);
                            if(mob <= 1){
                                obonus += RookSemiKingFileOpening;
                                if(mob == 0)
                                     obonus += RookKingFileOpening
                                                  - RookSemiKingFileOpening;
                            }
                        }
                    }
                    if(r == RANK_7){
                        if((bitboard[PAWN+opp] &
                            BB_RANK[RELATIVE_RANK[c][RANK_7]])
                            || kR == RELATIVE_RANK[c][RANK_8]){
                            obonus += Rook7th[Opening];
                            ebonus += Rook7th[Endgame];
                        }
                    }
                    if(r >= RANK_5){
                        att_num++;
                        att_value+= 2;
                    }
                    if(print_info){
                        std::cout<<std::setw(15)<<"* Rook ["<<piece_to_char(c)<<"] |";
                        std::cout<<std::setw(8)<<relative_value<c>(obonus + mob * RookMob[Opening])<<" |";
                        std::cout<<std::setw(8)<<relative_value<c>(ebonus + mob * RookMob[Endgame])<<" |"<<std::endl;
                    }
                    opening += obonus + mob * RookMob[Opening];
                    endgame += ebonus + mob * RookMob[Endgame];
                    break;
                }
                case QUEEN+c:{
                     sq_attack2 = sq_attack & king_safety2;
                     if(sq_attack2){
                        if(sq_attack & king_safety || bb_queen_attack(
                                sq,occupied ^ (occupied & sq_attack2))
                               & king_safety){
                           info.att_num[opp] ++;
                           info.att_total[opp] += KingAttackUnit[QUEEN];
                       }
                    }
                    else if(bb_rook_attack(sq,occupied^(bitboard[ROOK+c]))
                            & king_safety){
                        info.att_num[opp] ++;
                        info.att_total[opp] += KingAttackUnit[ROOK];
                    }
                    f = file(sq);
                    r = rank(sq);
                    penalty = 10 -(abs(kR-r)+abs(kF-f));
                    if(c == Black) r = RANK_8 - r;
                    obonus = penalty;
                    ebonus = penalty;
                    if(r == RANK_7){
                        if((bitboard[PAWN+opp]
                            & BB_RANK[RELATIVE_RANK[c][RANK_7]])
                            || kR == RELATIVE_RANK[c][RANK_8]){
                            obonus += Queen7th[Opening];
                            ebonus += Queen7th[Endgame];
                        }
                    }
                    if(r >= RANK_5){
                        att_num++;
                        att_value += 4;
                    }
                    if(print_info){
                        std::cout<<std::setw(15)<<"* Queen ["<<piece_to_char(c)<<"] |";
                        std::cout<<std::setw(8)<<relative_value<c>(obonus)<<" |";
                        std::cout<<std::setw(8)<<relative_value<c>(ebonus)<<" |"<<std::endl;
                    }
                    opening += obonus;
                    endgame += ebonus;
                    break;
                }
                default: { break; }
            }
        }
    }
    opening += SSpaceWeight[att_num] * att_value;

    if(print_info){
        std::cout<<"---------------------------------------"<<std::endl;
        std::cout<<std::setw(15)<<"Pieces ["<<piece_to_char(c)<<"] |";
        std::cout<<std::setw(8)<<relative_value<c>(opening)<<" |";
        std::cout<<std::setw(8)<<relative_value<c>(endgame)<<" |"<<std::endl;
    }

    info.eval[Opening] += relative_value<c>(opening);
    info.eval[Endgame] += relative_value<c>(endgame);
}

template <Color c, bool print_info>
void Position::eval_patterns(EvalInfo& info) const{
    const Color opp = c^1;
    int opening = 0, endgame = 0;
    if((piece[relative_sq<c>(A7)] == BISHOP+c
        && piece[relative_sq<c>(B6)] == PAWN+opp)
        || (piece[relative_sq<c>(B8)] == BISHOP+c
            && piece[relative_sq<c>(C7)] == PAWN+opp)){
        opening -= TrappedBishop;
        endgame -= TrappedBishop;
    }
    if((piece[relative_sq<c>(H7)] == BISHOP+c
        && piece[relative_sq<c>(G6)] == PAWN+opp)
        || (piece[relative_sq<c>(G8)] == BISHOP+c
            && piece[relative_sq<c>(F7)] == PAWN+opp)){
        opening -= TrappedBishop;
        endgame -= TrappedBishop;
    }
    if(piece[relative_sq<c>(A6)] == BISHOP+c
       && piece[relative_sq<c>(B5)] == PAWN+opp){
        opening -= TrappedBishop / 2;
        endgame -= TrappedBishop / 2;
    }
    if(piece[relative_sq<c>(H6)] == BISHOP+c
       && piece[relative_sq<c>(G5)] == PAWN+opp) {
        opening -= TrappedBishop / 2;
        endgame -= TrappedBishop / 2;
    }
    if(piece[relative_sq<c>(D2)] == PAWN+c
       && piece[relative_sq<c>(D3)] != NO_PIECE
       && piece[relative_sq<c>(C1)] == BISHOP+c) opening -= BlockedBishop;
    if(piece[relative_sq<c>(E2)] == PAWN+c
       && piece[relative_sq<c>(E3)] != NO_PIECE
       && piece[relative_sq<c>(F1)] == BISHOP+c) opening -= BlockedBishop;
    if((piece[relative_sq<c>(C1)] == KING+c // Speed up (bitboard)
        || piece[relative_sq<c>(B1)] == KING+c)
        && (piece[relative_sq<c>(A1)] == ROOK+c
            || piece[relative_sq<c>(A2)]== ROOK+c
            || piece[relative_sq<c>(B1)]== ROOK+c)) opening -= BlockedRook;
    if((piece[relative_sq<c>(F1)] == KING+c
        || piece[relative_sq<c>(G1)] == KING+c) // Speed up (bitboard)
        && (piece[relative_sq<c>(H1)] == ROOK+c
            || piece[relative_sq<c>(H2)] == ROOK+c
            || piece[relative_sq<c>(G1)] == ROOK+c)) opening -= BlockedRook;

    if(print_info){
        std::cout<<std::setw(15)<<"Patterns ["<<piece_to_char(c)<<"] |";
        std::cout<<std::setw(8)<<relative_value<c>(opening)<<" |";
        std::cout<<std::setw(8)<<relative_value<c>(endgame)<<" |"<<std::endl;
    }

    info.eval[Opening] += relative_value<c>(opening);
    info.eval[Endgame] += relative_value<c>(endgame);
}

template <Color c, bool print_info>
void Position::eval_pawns(PawnHash* ph) const{
    Bitboard neighbour1, neighbour2, bb_passed = 0;
    Square sq;
    File f;
    Rank r;
    const Color opp = c ^ 1;
    int opening = 0, endgame = 0;
    bool backward, doubled, passed, isolated, open, candidate, chain;

    Bitboard pawns, c_pawns, opp_pawns;
    pawns = c_pawns = bitboard[PAWN+c];
    opp_pawns = bitboard[PAWN+opp];

    while(pawns){
        sq = first_bit_clear(pawns);
        f = file(sq);
        r = rank(sq);
        backward = doubled = isolated = open = passed = candidate = false;
        chain = false;

        if(bit_le((BB_FILE[f] & c_pawns),r)) doubled = true;

        neighbour1 = bb_file(c_pawns,f-1) | bb_file(c_pawns,f+1);
        neighbour2 = (c_pawns | opp_pawns) & BB_FILE[f];

        if(!neighbour1) isolated = true;
        else{
            if(neighbour1 & (BB_RANK[relative_add<c>(r,-1)]
                             | BB_RANK[r]) & c_pawns)
                chain = true;
            if(!relative_bit_le_eq<c>(neighbour1,r)){
                backward = true;
                if((neighbour1 & BB_RANK[relative_add<c>(r,1)]) != 0){
                    if(!((neighbour2 & BB_RANK[relative_add<c>(r,1)])
                        | ((bb_file(opp_pawns,f-1)
                            | bb_file(opp_pawns,f+1))
                           & (BB_RANK[relative_add<c>(r,1)] |
                              BB_RANK[relative_add<c>(r,2)]))))
                        backward = false;
                }
                else if(r == relative_rank<c>(RANK_2)
                        && ((neighbour1 & BB_RANK[relative_add<c>(r,2)]) != 0)){
                    if(!((neighbour2 & (BB_RANK[relative_add<c>(r,1)]
                                        | BB_RANK[relative_add<c>(r,2)]))
                        | ((bb_file(opp_pawns,f-1)
                            | bb_file(opp_pawns,f+1))
                           & (BB_RANK[relative_add<c>(r,1)]
                              |BB_RANK[relative_add<c>(r,2)]
                              |BB_RANK[relative_add<c>(r,3)]))))
                            backward = false;
                }
            }
        }
        if(!relative_bit_gt<c>(neighbour2,r)){
            open = true;
            if(!relative_bit_gt<c>(bb_file(opp_pawns, f-1),r)
               && !relative_bit_gt<c>(bb_file(opp_pawns,f+1),r)){
                passed = true;
                bb_passed |= Bitboard(1)<<sq;
             }else{
                int n = relative_bit_num_le_eq<c>(bb_file(c_pawns,f-1),r);
                n += relative_bit_num_le_eq<c>(bb_file(c_pawns,f+1),r);
                n -= relative_bit_num_gt<c>(bb_file(opp_pawns,f-1),r);
                n -= relative_bit_num_gt<c>(bb_file(opp_pawns,f+1),r);
                if(n>=0){
                    n = bit_num_eq(bb_file(c_pawns,f-1),relative_add<c>(r,-1));
                    n += bit_num_eq(bb_file(c_pawns,f+1),relative_add<c>(r,-1));
                    n -= bit_num_eq(bb_file(opp_pawns,f-1),relative_add<c>(r,1));
                    n -= bit_num_eq(bb_file(opp_pawns,f+1),relative_add<c>(r,1));
                    if(n >= 0) candidate = true;
                }
             }
        }

        if (isolated){
            if(open){
                opening -= IsolatedOpenOpening;
                endgame -= Isolated[Endgame];
            }else{
                opening -= Isolated[Opening];
                endgame -= Isolated[Endgame];
            }
        }
        else {
            if(chain){
                opening += 3;
                endgame += 3;
            }
            if(backward){
                if(open){
                    opening -= BackwardOpenOpening;
                    endgame -= Backward[Endgame];
                }else{
                    opening -= Backward[Opening];
                    endgame -= Backward[Endgame];
                }
            }
        }

        if(doubled){
            opening -= Doubled[Opening];
            endgame -= Doubled[Endgame];
        }

        if(candidate) {
            opening += quad(CandidateMin[Opening],CandidateMax[Opening],
                            relative_rank<c>(r));
            endgame += quad(CandidateMin[Endgame],CandidateMax[Endgame],
                            relative_rank<c>(r));
        }
    }

    if(print_info){
        std::cout<<std::setw(15)<<"Pawns ["<<piece_to_char(c)<<"] |";
        std::cout<<std::setw(8)<<relative_value<c>(opening)<<" |";
        std::cout<<std::setw(8)<<relative_value<c>(endgame)<<" |"<<std::endl;
    }

    ph->key = current->pawn_key;
    ph->passed[c] = bb_passed;
    ph->eval[Opening] += relative_value<c>(opening);
    ph->eval[Endgame] += relative_value<c>(endgame);
}

template <Color c, bool print_info>
void Position::eval_king(EvalInfo& info, const MaterialHash* mh, int & margin) const{
    int opening = 0;
    if(mh->cflags[c] & MatKingFlag){
        int penalty = 0, penalty1, penalty2, tmp;
        const Square kSq = first_bit(bitboard[KING+c]);
        const Color opp = c^1;
        if(bitboard[QUEEN+(c ^ 1)] != 0){
            penalty1 = shelter_sq<c>(kSq);
            penalty2 = penalty1;
            if((current->castle & RELATIVE_CASTLE[c][WK_CASTLE]) != 0){
                tmp = shelter_sq<c>(relative_sq<c>(G1));
                if(tmp < penalty2) penalty2 = tmp;
            }
            if((current->castle & RELATIVE_CASTLE[c][WQ_CASTLE]) != 0){
                tmp = shelter_sq<c>(relative_sq<c>(B1));
                if(tmp < penalty2) penalty2 = tmp;
            }
            penalty = (penalty1+penalty2)/2;
        }
        const File kF = file(kSq);
        penalty += storm_file(kF,c);
        if(kF != FILE_A) penalty += storm_file(kF-1,c);
        if(kF != FILE_H) penalty += storm_file(kF+1,c);

        info.attacked_by[KING+c] = KingAttack[kSq];
        Bitboard undefended = info.attacked_by[opp] & info.attacked_by[KING+c];
        undefended &= ~( info.attacked_by[PAWN+c] | info.attacked_by[KNIGHT+c]
                        | info.attacked_by[BISHOP+c] | info.attacked_by[ROOK+c]
                        | info.attacked_by[QUEEN+c]);

        Bitboard b = undefended & info.attacked_by[QUEEN+opp] & ~bitboard[opp];
        if(b){
            Bitboard b2 = b;
            b &= (info.attacked_by[PAWN+opp] | info.attacked_by[KNIGHT+opp]
                  | info.attacked_by[BISHOP+opp] | info.attacked_by[ROOK+opp]);
            if(b) info.att_total[c] += KingAttackUnit[QUEEN]
                                       * bit_count_3(b)
                                       * (opp == stm ? 2 : 1);
            else{
                Square sq;
                Bitboard p = bitboard[BISHOP+opp];
                while(p){
                    sq = first_bit_clear(p);
                    b = b2 & bb_bishop_attack(sq,occupied^bitboard[QUEEN+opp]);
                    if(b) info.att_total[c] += KingAttackUnit[QUEEN]
                                             * bit_count_3(b)
                                             * (opp == stm ? 2 : 1);
                }
                p = bitboard[ROOK+opp];
                while(p){
                    sq = first_bit_clear(p);
                    b = b2 & bb_rook_attack(sq,occupied^bitboard[ROOK+opp]);
                    if(b) info.att_total[c] += KingAttackUnit[QUEEN]
                                             * bit_count_3(b)
                                             * (opp == stm ? 2 : 1);
                }
            }
        }
        b = undefended & info.attacked_by[ROOK+opp] & ~bitboard[opp];
        b &= KingAttackRook[kSq];

        if(b){
            Bitboard b2 = b;
            b &= (info.attacked_by[PAWN+opp] | info.attacked_by[KNIGHT+opp]
                  | info.attacked_by[BISHOP+opp] | info.attacked_by[QUEEN+opp]);
            if (b) info.att_total[c] += KingAttackUnit[ROOK]
                                        * bit_count_3(b)
                                        * (opp == stm ? 2 : 1);
            else{
                Square sq;
                Bitboard p = bitboard[QUEEN+opp];
                while(p){
                    sq = first_bit_clear(p);
                    b = b2 & bb_rook_attack(sq,occupied^bitboard[QUEEN+opp]);
                    if(b) info.att_total[c] += KingAttackUnit[ROOK]
                                             * bit_count_3(b)
                                             * (opp == stm ? 2 : 1);
                }
                p = bitboard[ROOK+opp];
                while(p){
                    sq = first_bit_clear(p);
                    if(!(b2 & bb_rook_attack(sq,occupied))){
                        b = b2 & bb_rook_attack(sq,occupied^bitboard[QUEEN+opp]);
                        if(b) info.att_total[c] += KingAttackUnit[ROOK]
                                                 * bit_count_3(b)
                                                 * (opp == stm ? 2 : 1);
                    }
                }
            }
        }

        const Bitboard safe = ~(bitboard[opp] | info.attacked_by[c]);

        Bitboard b1 = rook_attack(kSq) & safe;
        Bitboard b2 = bishop_attack(kSq) & safe;

        b = (b1 | b2) & info.attacked_by[QUEEN+opp];
        if (b) info.att_total[c] += bit_count_15(b) * KingAttackUnit[QUEEN];

        b = b1 & info.attacked_by[ROOK+opp];
        if (b) info.att_total[c] += bit_count_3(b) * KingAttackUnit[ROOK];

        b = b2 & info.attacked_by[BISHOP+opp];
        if (b) info.att_total[c] += bit_count_3(b) * KingAttackUnit[BISHOP];

        b = knight_attack(kSq) & info.attacked_by[KNIGHT+opp] & safe;
        if (b) info.att_total[c] += bit_count_3(b) * KingAttackUnit[KNIGHT];

        opening -= ((penalty * ShelterOpening) +
                    (info.att_total[c] * KingAttackOpening
                     * KingAttackWeight[info.att_num[c]])) / 256;
    }

    if(c != stm) margin -= opening;

    if(print_info){
        std::cout<<std::setw(15)<<"King ["<<piece_to_char(c)<<"] |";
        std::cout<<std::setw(8)<<relative_value<c>(opening)<<" |";
        std::cout<<std::setw(8)<<0<<" |"<<std::endl;
    }

    info.eval[Opening] += relative_value<c>(opening);
}

template <Color c, bool print_info>
void Position::eval_threats(EvalInfo& info) const{
    Bitboard weak_pieces, bb;
    int opening = 0, endgame = 0;
    const Color opp = c ^ 1;
    weak_pieces = bitboard[opp] & ~info.attacked_by[PAWN+opp]
                  & info.attacked_by[c];

    if(!weak_pieces) return;
    for(Piece p1 = KNIGHT+c; p1 < KING+c; p1+=2){
        bb = info.attacked_by[p1] & weak_pieces;
        if(bb){
            for (Piece p2 = PAWN+opp; p2 < KING+opp; p2+=2){
                if (bb & bitboard[p2]){
                    opening += ThreatBonus[p1][p2][Opening];
                    endgame += ThreatBonus[p1][p2][Endgame];
                }
            }
        }
    }

    if(print_info){
        std::cout<<std::setw(15)<<"Threats ["<<piece_to_char(c)<<"] |";
        std::cout<<std::setw(8)<<relative_value<c>(opening)<<" |";
        std::cout<<std::setw(8)<<relative_value<c>(endgame)<<" |"<<std::endl;
    }


    info.eval[Opening] += relative_value<c>(opening);
    info.eval[Endgame] += relative_value<c>(endgame);
}

template <Color c, bool print_info>
void Position::eval_passer(EvalInfo& info, const PawnHash* ph) const {
    Bitboard bb,bb_to_q,bb_behind,bb_defended,bb_unsafe,bb_safe;
    Square sq, b_sq, b_sq2;
    Rank r;
    File f;
    int opening = 0, endgame = 0, r2, rf, bonus[2];
    const Color opp = c^1;
    const Square k_sq = first_bit(bitboard[KING+c]);
    const Square opp_k_sq = first_bit(bitboard[KING+opp]);

    bb = ph->passed[c];
    while(bb){
        sq = first_bit_clear(bb);
        r = rank(relative_sq<c>(sq));
        f = file(sq);
        r2 = r - RANK_2;
        rf = r2 * (r2 - 1);

        bonus[Opening] = 20 * rf;
        bonus[Endgame] = 10 * (rf + r2 + 1);

        if(rf){
            if(c == White){
                b_sq = sq + 8;
                b_sq2 = sq + 16;
            }
            else{
                b_sq = sq - 8;
                b_sq2 = sq - 16;
            }

            bonus[Endgame] -= distance(k_sq, b_sq) * 3 * rf;
            bonus[Endgame] -= distance(k_sq, b_sq2) * 1 * rf;
            bonus[Endgame] += distance(opp_k_sq, b_sq) * 6 * rf;

            if(((bitboard[KNIGHT + opp] | bitboard[BISHOP + opp]
                 | bitboard[ROOK + opp] | bitboard[QUEEN + opp]) == 0)
                && (unstoppable_passer(sq,c) || king_passer(sq,c))){
                    endgame += quad(0,800,r);
            }
            else if(piece[b_sq] == NO_PIECE){
                if(c == White){
                    bb_to_q = bb_gt(BB_FILE[f],r);
                    bb_behind = bb_le(BB_FILE[f],r);
                }
                else{
                    bb_to_q = bb_le(BB_FILE[f],r);
                    bb_behind = bb_gt(BB_FILE[f],r);
                }
                bb_defended = bb_to_q & info.attacked_by[c];
                bb_unsafe = bb_safe = 0;
                if(bb_behind & (bitboard[QUEEN+opp] | bitboard[ROOK+opp])
                   & rook_attack(sq)){
                   bb_unsafe = bb_to_q;
                }
                else{
                    bb_unsafe = bb_to_q & (info.attacked_by[opp]
                                           | bitboard[opp]);
                }
                if(!bb_unsafe){
                    if(bb_to_q == bb_defended) bonus[Endgame] += rf * 17;
                    else bonus[Endgame] += rf * 15;
                }
                else{
                    if((bb_unsafe & bb_defended) == bb_unsafe)
                        bonus[Endgame] += rf * 13;
                    else bonus[Endgame] += rf * 8;
                }
                if((bb_to_q & bitboard[c]) == 0) bonus[Endgame] += rf;
            }
        }

        Bitboard neighbour = bitboard[PAWN+c] & ( bb_file(bitboard[c],f-1)
                                                | bb_file(bitboard[c],f+1));
        if((neighbour & BB_RANK[r]) != 0){
            bonus[Endgame] += r2 * 20;
        }else{
            if(c == White){
                if((neighbour & BB_RANK[r - 1]) != 0){
                    bonus[Endgame] += r2 * 12;
                }
            }else{
                if((neighbour & BB_RANK[(RANK_8 - r) + 1]) != 0){
                    bonus[Endgame] += r2 * 12;
                }
            }
        }

        if(f == FILE_A || f == FILE_H){
            if(num[KNIGHT + opp] <= 1
               && num[BISHOP + opp]+num[ROOK + opp]+num[QUEEN + opp] == 0)
                bonus[Endgame] += bonus[Endgame] / 4;
            else if(num[QUEEN + opp] + num[ROOK + opp] != 0)
                bonus[Endgame] -= bonus[Endgame] / 4;
        }

        opening += bonus[Opening] / 2;
        endgame += bonus[Endgame] / 2;
    }

    if(print_info){
        std::cout<<std::setw(15)<<"Passer pawns ["<<piece_to_char(c)<<"] |";
        std::cout<<std::setw(8)<<relative_value<c>(opening)<<" |";
        std::cout<<std::setw(8)<<relative_value<c>(endgame)<<" |"<<std::endl;
    }

    info.eval[Opening] += relative_value<c>(opening);
    info.eval[Endgame] += relative_value<c>(endgame);
}

inline int Position::quad(const int min, const int max, const int x) {
    return  min + ((max - min) * Bonus[x] + 128) / 256;
}

template <Color c>
inline int Position::shelter_sq(const Square sq) const{
    const int f = file(sq);
    const int r = rank(sq);
    int penalty = shelter_file(f,r,c) * 2;
    if (f != FILE_A) penalty += shelter_file(f-1,r,c);
    if (f != FILE_H) penalty += shelter_file(f+1,r,c);

    if (penalty == 0) penalty = 11;

    penalty += storm_file(f,c);
    if (f != FILE_A) penalty += storm_file(f-1,c);
    if (f != FILE_H) penalty += storm_file(f+1,c);

    return penalty;
}

inline int Position::shelter_file(const int f,const int r,const Color c)const{
    int distance, penalty;
    Bitboard bb = bitboard[PAWN+c] & BB_FILE[f];

    if(c == White){
        distance = r*8;
        bb = (bb >> distance)<<distance;
        if(bb) distance = RANK_8 - rank(first_bit(bb));
        else distance = 0;
    }
    else{
        distance = (RANK_8-r)*8;
        bb = (bb << distance)>>distance;
        if(bb) distance = rank(last_bit(bb));
        else distance = 0;
    }
    penalty = 36 - distance * distance;
    return penalty;
}

inline int Position::storm_file(const int f, const Color c) const{
    int distance = 0, penalty = 0;
    Bitboard bb = bitboard[PAWN+(c^1)] & BB_FILE[f];

   if(c == White){
        if(bb) distance = RANK_8 - rank(first_bit(bb));
        else distance = 0;
    }
    else{
        if(bb) distance = rank(last_bit(bb));
        else distance = 0;
    }
    switch (distance) {
        case RANK_4:{ penalty = StormOpening * 1; break;}
        case RANK_5:{ penalty = StormOpening * 2; break;}
        case RANK_6:{ penalty = StormOpening * 3; break;}
    }

    return penalty;
}

inline bool Position::unstoppable_passer(Square sq, const Color c) const{
    Square opp_kSq;
    int dist;
    Rank r = rank(sq);

    if(c == White){
        opp_kSq = first_bit(bitboard[BK]);
        if(bit_gt(BB_FILE[file(sq)] & bitboard[White],r))
            return false;
        if(r == RANK_2){
            r ++;
            sq += 8;
        }
        dist = distance(opp_kSq,sq+(8*(RANK_8-r)));
        if(stm == Black) dist --;
        if(dist > RANK_8 - r) return true;
    }
    else{
        opp_kSq = first_bit(bitboard[WK]);
        if(bit_le(BB_FILE[file(sq)] & bitboard[Black],r))
            return false;
        if(r == RANK_7){
            r --;
            sq -= 8;
        }
        dist = distance(opp_kSq,sq-(8*r));
        if(stm == White) dist --;
        if(dist > r) return true;
    }
    return false;
}

inline bool Position::king_passer(const Square sq, const Color c) const{
    Square pSq;
    const Square kSq =  first_bit(bitboard[KING+c]);
    const File f = file(sq);
    if(c == White) pSq = kSq + (8*(RANK_8-rank(kSq)));
    else pSq = kSq - (8*rank(kSq));
    if(distance(kSq,pSq) <= 1 && distance(kSq,sq) == 1
       && (file(kSq) != f || (f != FILE_A && f != FILE_H))){
        return true;
    }
    return false;
}

inline bool Position::bishop_can_attack(const Square to, const Color c) const{
    Bitboard bb = bitboard[BISHOP+c];
    Square sq;
    while(bb){
        sq = first_bit_clear(bb);
        if(color(sq) == color(to)){
            return true;
        }
    }
    return false;
}

void Position::print_eval(Thread & thread) const{
    int margin;
    eval<false>(thread,margin);
    std::cout<<"                  | Opening | Endgame |"<<std::endl;
    std::cout<<"---------------------------------------"<<std::endl;
    eval<true>(thread,margin);
}
