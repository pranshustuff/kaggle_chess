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

#include <iostream>
#include "bitboard.h"
#include "piece.h"

void bb_print(Bitboard x){
    int i;
    Square sq;
    char stype[72];
    for(i=0; i<71; i++) stype[i] = '0';
    for(i=8; i<71; i+=9) stype[i] = '\n';
    stype[71] = '\0';
    while(x != 0){
        sq = first_bit_clear(x);
        stype[70-rank(sq)*9-(7-file(sq))] = '1';
    }
    std::cout<<stype<<std::endl;
}

#if !defined(__i386__) && !defined(__x86_64__)

static const Square BitTable[64] = {
    Square(0), Square(1), Square(2), Square(7),
    Square(3), Square(13), Square(8), Square(19),
    Square(4), Square(25), Square(14), Square(28),
    Square(9), Square(34), Square(20), Square(40),
    Square(5), Square(17), Square(26), Square(38),
    Square(15), Square(46), Square(29), Square(48),
    Square(10), Square(31), Square(35), Square(54),
    Square(21), Square(50), Square(41), Square(57),
    Square(63), Square(6), Square(12), Square(18),
    Square(24), Square(27), Square(33), Square(39),
    Square(16), Square(37), Square(45), Square(47),
    Square(30), Square(53), Square(49), Square(56),
    Square(62), Square(11), Square(23), Square(32),
    Square(36), Square(44), Square(52), Square(55),
    Square(61), Square(22), Square(43), Square(51),
    Square(60), Square(42), Square(59), Square(58)
};

Square first_bit(Bitboard b) {
    return BitTable[((b & -b) * 0x218a392cd3d5dbfULL) >> 58];
}

Square last_bit(Bitboard b){
    Square sq = (Square)63;
    if (!(b & 0xffffffff00000000ull)){b <<= 32; sq -= 32;}
    if (!(b & 0xffff000000000000ull)){b <<= 16; sq -= 16;}
    if (!(b & 0xff00000000000000ull)){b <<= 8; sq -= 8;}
    if (!(b & 0xf000000000000000ull)){b <<= 4; sq -= 4;}
    if (!(b & 0xc000000000000000ull)){b <<= 2; sq -= 2;}
    if (!(b & 0x8000000000000000ull)){b <<= 1; sq -= 1;}
    return sq;
}

Square first_bit_clear(Bitboard& b) {
    Bitboard bb = b;
    b &= (b - 1);
    return BitTable[((bb & -bb) * 0x218a392cd3d5dbfULL) >> 58];
}

#endif

const int magic_bb_r_shift[64] = {
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 53, 53, 53, 53, 53
};

const Bitboard magic_bb_r_magics[64] = {
    Bitboard(0x0080001020400080ull), Bitboard(0x0040001000200040ull),
    Bitboard(0x0080081000200080ull), Bitboard(0x0080040800100080ull),
    Bitboard(0x0080020400080080ull), Bitboard(0x0080010200040080ull),
    Bitboard(0x0080008001000200ull), Bitboard(0x0080002040800100ull),
    Bitboard(0x0000800020400080ull), Bitboard(0x0000400020005000ull),
    Bitboard(0x0000801000200080ull), Bitboard(0x0000800800100080ull),
    Bitboard(0x0000800400080080ull), Bitboard(0x0000800200040080ull),
    Bitboard(0x0000800100020080ull), Bitboard(0x0000800040800100ull),
    Bitboard(0x0000208000400080ull), Bitboard(0x0000404000201000ull),
    Bitboard(0x0000808010002000ull), Bitboard(0x0000808008001000ull),
    Bitboard(0x0000808004000800ull), Bitboard(0x0000808002000400ull),
    Bitboard(0x0000010100020004ull), Bitboard(0x0000020000408104ull),
    Bitboard(0x0000208080004000ull), Bitboard(0x0000200040005000ull),
    Bitboard(0x0000100080200080ull), Bitboard(0x0000080080100080ull),
    Bitboard(0x0000040080080080ull), Bitboard(0x0000020080040080ull),
    Bitboard(0x0000010080800200ull), Bitboard(0x0000800080004100ull),
    Bitboard(0x0000204000800080ull), Bitboard(0x0000200040401000ull),
    Bitboard(0x0000100080802000ull), Bitboard(0x0000080080801000ull),
    Bitboard(0x0000040080800800ull), Bitboard(0x0000020080800400ull),
    Bitboard(0x0000020001010004ull), Bitboard(0x0000800040800100ull),
    Bitboard(0x0000204000808000ull), Bitboard(0x0000200040008080ull),
    Bitboard(0x0000100020008080ull), Bitboard(0x0000080010008080ull),
    Bitboard(0x0000040008008080ull), Bitboard(0x0000020004008080ull),
    Bitboard(0x0000010002008080ull), Bitboard(0x0000004081020004ull),
    Bitboard(0x0000204000800080ull), Bitboard(0x0000200040008080ull),
    Bitboard(0x0000100020008080ull), Bitboard(0x0000080010008080ull),
    Bitboard(0x0000040008008080ull), Bitboard(0x0000020004008080ull),
    Bitboard(0x0000800100020080ull), Bitboard(0x0000800041000080ull),
    Bitboard(0x00FFFCDDFCED714Aull), Bitboard(0x007FFCDDFCED714Aull),
    Bitboard(0x003FFFCDFFD88096ull), Bitboard(0x0000040810002101ull),
    Bitboard(0x0001000204080011ull), Bitboard(0x0001000204000801ull),
    Bitboard(0x0001000082000401ull), Bitboard(0x0001FFFAABFAD1A2ull)
};

const Bitboard magic_bb_r_mask[64] = {
    Bitboard(0x000101010101017Eull), Bitboard(0x000202020202027Cull),
    Bitboard(0x000404040404047Aull), Bitboard(0x0008080808080876ull),
    Bitboard(0x001010101010106Eull), Bitboard(0x002020202020205Eull),
    Bitboard(0x004040404040403Eull), Bitboard(0x008080808080807Eull),
    Bitboard(0x0001010101017E00ull), Bitboard(0x0002020202027C00ull),
    Bitboard(0x0004040404047A00ull), Bitboard(0x0008080808087600ull),
    Bitboard(0x0010101010106E00ull), Bitboard(0x0020202020205E00ull),
    Bitboard(0x0040404040403E00ull), Bitboard(0x0080808080807E00ull),
    Bitboard(0x00010101017E0100ull), Bitboard(0x00020202027C0200ull),
    Bitboard(0x00040404047A0400ull), Bitboard(0x0008080808760800ull),
    Bitboard(0x00101010106E1000ull), Bitboard(0x00202020205E2000ull),
    Bitboard(0x00404040403E4000ull), Bitboard(0x00808080807E8000ull),
    Bitboard(0x000101017E010100ull), Bitboard(0x000202027C020200ull),
    Bitboard(0x000404047A040400ull), Bitboard(0x0008080876080800ull),
    Bitboard(0x001010106E101000ull), Bitboard(0x002020205E202000ull),
    Bitboard(0x004040403E404000ull), Bitboard(0x008080807E808000ull),
    Bitboard(0x0001017E01010100ull), Bitboard(0x0002027C02020200ull),
    Bitboard(0x0004047A04040400ull), Bitboard(0x0008087608080800ull),
    Bitboard(0x0010106E10101000ull), Bitboard(0x0020205E20202000ull),
    Bitboard(0x0040403E40404000ull), Bitboard(0x0080807E80808000ull),
    Bitboard(0x00017E0101010100ull), Bitboard(0x00027C0202020200ull),
    Bitboard(0x00047A0404040400ull), Bitboard(0x0008760808080800ull),
    Bitboard(0x00106E1010101000ull), Bitboard(0x00205E2020202000ull),
    Bitboard(0x00403E4040404000ull), Bitboard(0x00807E8080808000ull),
    Bitboard(0x007E010101010100ull), Bitboard(0x007C020202020200ull),
    Bitboard(0x007A040404040400ull), Bitboard(0x0076080808080800ull),
    Bitboard(0x006E101010101000ull), Bitboard(0x005E202020202000ull),
    Bitboard(0x003E404040404000ull), Bitboard(0x007E808080808000ull),
    Bitboard(0x7E01010101010100ull), Bitboard(0x7C02020202020200ull),
    Bitboard(0x7A04040404040400ull), Bitboard(0x7608080808080800ull),
    Bitboard(0x6E10101010101000ull), Bitboard(0x5E20202020202000ull),
    Bitboard(0x3E40404040404000ull), Bitboard(0x7E80808080808000ull)
};

const int magic_bb_b_shift[64] = {
    58, 59, 59, 59, 59, 59, 59, 58,
    59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 59, 59, 59, 59, 59, 59, 58
};

const Bitboard magic_bb_b_magics[64] = {
    Bitboard(0x0002020202020200ull), Bitboard(0x0002020202020000ull),
    Bitboard(0x0004010202000000ull), Bitboard(0x0004040080000000ull),
    Bitboard(0x0001104000000000ull), Bitboard(0x0000821040000000ull),
    Bitboard(0x0000410410400000ull), Bitboard(0x0000104104104000ull),
    Bitboard(0x0000040404040400ull), Bitboard(0x0000020202020200ull),
    Bitboard(0x0000040102020000ull), Bitboard(0x0000040400800000ull),
    Bitboard(0x0000011040000000ull), Bitboard(0x0000008210400000ull),
    Bitboard(0x0000004104104000ull), Bitboard(0x0000002082082000ull),
    Bitboard(0x0004000808080800ull), Bitboard(0x0002000404040400ull),
    Bitboard(0x0001000202020200ull), Bitboard(0x0000800802004000ull),
    Bitboard(0x0000800400A00000ull), Bitboard(0x0000200100884000ull),
    Bitboard(0x0000400082082000ull), Bitboard(0x0000200041041000ull),
    Bitboard(0x0002080010101000ull), Bitboard(0x0001040008080800ull),
    Bitboard(0x0000208004010400ull), Bitboard(0x0000404004010200ull),
    Bitboard(0x0000840000802000ull), Bitboard(0x0000404002011000ull),
    Bitboard(0x0000808001041000ull), Bitboard(0x0000404000820800ull),
    Bitboard(0x0001041000202000ull), Bitboard(0x0000820800101000ull),
    Bitboard(0x0000104400080800ull), Bitboard(0x0000020080080080ull),
    Bitboard(0x0000404040040100ull), Bitboard(0x0000808100020100ull),
    Bitboard(0x0001010100020800ull), Bitboard(0x0000808080010400ull),
    Bitboard(0x0000820820004000ull), Bitboard(0x0000410410002000ull),
    Bitboard(0x0000082088001000ull), Bitboard(0x0000002011000800ull),
    Bitboard(0x0000080100400400ull), Bitboard(0x0001010101000200ull),
    Bitboard(0x0002020202000400ull), Bitboard(0x0001010101000200ull),
    Bitboard(0x0000410410400000ull), Bitboard(0x0000208208200000ull),
    Bitboard(0x0000002084100000ull), Bitboard(0x0000000020880000ull),
    Bitboard(0x0000001002020000ull), Bitboard(0x0000040408020000ull),
    Bitboard(0x0004040404040000ull), Bitboard(0x0002020202020000ull),
    Bitboard(0x0000104104104000ull), Bitboard(0x0000002082082000ull),
    Bitboard(0x0000000020841000ull), Bitboard(0x0000000000208800ull),
    Bitboard(0x0000000010020200ull), Bitboard(0x0000000404080200ull),
    Bitboard(0x0000040404040400ull), Bitboard(0x0002020202020200ull)
};

const Bitboard magic_bb_b_mask[64] = {
    Bitboard(0x0040201008040200ull), Bitboard(0x0000402010080400ull),
    Bitboard(0x0000004020100A00ull), Bitboard(0x0000000040221400ull),
    Bitboard(0x0000000002442800ull), Bitboard(0x0000000204085000ull),
    Bitboard(0x0000020408102000ull), Bitboard(0x0002040810204000ull),
    Bitboard(0x0020100804020000ull), Bitboard(0x0040201008040000ull),
    Bitboard(0x00004020100A0000ull), Bitboard(0x0000004022140000ull),
    Bitboard(0x0000000244280000ull), Bitboard(0x0000020408500000ull),
    Bitboard(0x0002040810200000ull), Bitboard(0x0004081020400000ull),
    Bitboard(0x0010080402000200ull), Bitboard(0x0020100804000400ull),
    Bitboard(0x004020100A000A00ull), Bitboard(0x0000402214001400ull),
    Bitboard(0x0000024428002800ull), Bitboard(0x0002040850005000ull),
    Bitboard(0x0004081020002000ull), Bitboard(0x0008102040004000ull),
    Bitboard(0x0008040200020400ull), Bitboard(0x0010080400040800ull),
    Bitboard(0x0020100A000A1000ull), Bitboard(0x0040221400142200ull),
    Bitboard(0x0002442800284400ull), Bitboard(0x0004085000500800ull),
    Bitboard(0x0008102000201000ull), Bitboard(0x0010204000402000ull),
    Bitboard(0x0004020002040800ull), Bitboard(0x0008040004081000ull),
    Bitboard(0x00100A000A102000ull), Bitboard(0x0022140014224000ull),
    Bitboard(0x0044280028440200ull), Bitboard(0x0008500050080400ull),
    Bitboard(0x0010200020100800ull), Bitboard(0x0020400040201000ull),
    Bitboard(0x0002000204081000ull), Bitboard(0x0004000408102000ull),
    Bitboard(0x000A000A10204000ull), Bitboard(0x0014001422400000ull),
    Bitboard(0x0028002844020000ull), Bitboard(0x0050005008040200ull),
    Bitboard(0x0020002010080400ull), Bitboard(0x0040004020100800ull),
    Bitboard(0x0000020408102000ull), Bitboard(0x0000040810204000ull),
    Bitboard(0x00000A1020400000ull), Bitboard(0x0000142240000000ull),
    Bitboard(0x0000284402000000ull), Bitboard(0x0000500804020000ull),
    Bitboard(0x0000201008040200ull), Bitboard(0x0000402010080400ull),
    Bitboard(0x0002040810204000ull), Bitboard(0x0004081020400000ull),
    Bitboard(0x000A102040000000ull), Bitboard(0x0014224000000000ull),
    Bitboard(0x0028440200000000ull), Bitboard(0x0050080402000000ull),
    Bitboard(0x0020100804020000ull), Bitboard(0x0040201008040200ull)
};

const Bitboard* magic_bb_b_indices[64] = {
    magic_bb_b_db+4992, magic_bb_b_db+2624, magic_bb_b_db+256,
    magic_bb_b_db+896,  magic_bb_b_db+1280, magic_bb_b_db+1664,
    magic_bb_b_db+4800, magic_bb_b_db+5120, magic_bb_b_db+2560,
    magic_bb_b_db+2656, magic_bb_b_db+288,  magic_bb_b_db+928,
    magic_bb_b_db+1312, magic_bb_b_db+1696, magic_bb_b_db+4832,
    magic_bb_b_db+4928, magic_bb_b_db+0,    magic_bb_b_db+128,
    magic_bb_b_db+320,  magic_bb_b_db+960,  magic_bb_b_db+1344,
    magic_bb_b_db+1728, magic_bb_b_db+2304, magic_bb_b_db+2432,
    magic_bb_b_db+32,   magic_bb_b_db+160,  magic_bb_b_db+448,
    magic_bb_b_db+2752, magic_bb_b_db+3776, magic_bb_b_db+1856,
    magic_bb_b_db+2336, magic_bb_b_db+2464, magic_bb_b_db+64,
    magic_bb_b_db+192,  magic_bb_b_db+576,  magic_bb_b_db+3264,
    magic_bb_b_db+4288, magic_bb_b_db+1984, magic_bb_b_db+2368,
    magic_bb_b_db+2496, magic_bb_b_db+96,   magic_bb_b_db+224,
    magic_bb_b_db+704,  magic_bb_b_db+1088, magic_bb_b_db+1472,
    magic_bb_b_db+2112, magic_bb_b_db+2400, magic_bb_b_db+2528,
    magic_bb_b_db+2592, magic_bb_b_db+2688, magic_bb_b_db+832,
    magic_bb_b_db+1216, magic_bb_b_db+1600, magic_bb_b_db+2240,
    magic_bb_b_db+4864, magic_bb_b_db+4960, magic_bb_b_db+5056,
    magic_bb_b_db+2720, magic_bb_b_db+864,  magic_bb_b_db+1248,
    magic_bb_b_db+1632, magic_bb_b_db+2272, magic_bb_b_db+4896,
    magic_bb_b_db+5184
};

const Bitboard* magic_bb_r_indices[64] = {
    magic_bb_r_db+86016, magic_bb_r_db+73728, magic_bb_r_db+36864,
    magic_bb_r_db+43008, magic_bb_r_db+47104, magic_bb_r_db+51200,
    magic_bb_r_db+77824, magic_bb_r_db+94208, magic_bb_r_db+69632,
    magic_bb_r_db+32768, magic_bb_r_db+38912, magic_bb_r_db+10240,
    magic_bb_r_db+14336, magic_bb_r_db+53248, magic_bb_r_db+57344,
    magic_bb_r_db+81920, magic_bb_r_db+24576, magic_bb_r_db+33792,
    magic_bb_r_db+6144,  magic_bb_r_db+11264, magic_bb_r_db+15360,
    magic_bb_r_db+18432, magic_bb_r_db+58368, magic_bb_r_db+61440,
    magic_bb_r_db+26624, magic_bb_r_db+4096,  magic_bb_r_db+7168,
    magic_bb_r_db+0,     magic_bb_r_db+2048,  magic_bb_r_db+19456,
    magic_bb_r_db+22528, magic_bb_r_db+63488, magic_bb_r_db+28672,
    magic_bb_r_db+5120,  magic_bb_r_db+8192,  magic_bb_r_db+1024,
    magic_bb_r_db+3072,  magic_bb_r_db+20480, magic_bb_r_db+23552,
    magic_bb_r_db+65536, magic_bb_r_db+30720, magic_bb_r_db+34816,
    magic_bb_r_db+9216,  magic_bb_r_db+12288, magic_bb_r_db+16384,
    magic_bb_r_db+21504, magic_bb_r_db+59392, magic_bb_r_db+67584,
    magic_bb_r_db+71680, magic_bb_r_db+35840, magic_bb_r_db+39936,
    magic_bb_r_db+13312, magic_bb_r_db+17408, magic_bb_r_db+54272,
    magic_bb_r_db+60416, magic_bb_r_db+83968, magic_bb_r_db+90112,
    magic_bb_r_db+75776, magic_bb_r_db+40960, magic_bb_r_db+45056,
    magic_bb_r_db+49152, magic_bb_r_db+55296, magic_bb_r_db+79872,
    magic_bb_r_db+98304
};

Bitboard magic_bb_r_db[102400];
Bitboard magic_bb_b_db[5248];

Bitboard PawnAttack[2][64];
Bitboard KnightAttack[64];
Bitboard KingAttack[64];
Bitboard KingAttack2[64];
Bitboard KingAttackRook[64];
Bitboard BishopAttack[64];
Bitboard RookAttack[64];
Bitboard QueenAttack[64];
Bitboard BBColorSquare[2];

AttackFunctionBB bb_attack[14];

void init_attack();
void init_magic_bb();
Bitboard init_magic_bb_r(const Square sq, const Bitboard occ);
Bitboard init_magic_bb_occ(const Square* sq, const int numSq,
                           const Bitboard linocc);
Bitboard init_magic_bb_b(const Square sq, const Bitboard occ);

void bb_init(){
    init_attack();
    init_magic_bb();
}

void init_attack(){
    Bitboard mask;
    for(Square sq=A1; sq<=H8; sq++) {
                mask = Bitboard(1)<<sq;
        PawnAttack[White][sq]= shift_up_left(mask) | shift_up_right(mask);
        PawnAttack[Black][sq]= shift_down_left(mask) | shift_down_right(mask);
        BishopAttack[sq] = bb_bishop_attack(sq,0);
        RookAttack[sq] = bb_rook_attack(sq,0);
        QueenAttack[sq] = bb_queen_attack(sq,0);
    }
    BBColorSquare[White] = BBColorSquare[Black] = 0;

    for(Square sq=A1; sq<=H8; sq++){
        mask = Bitboard(1)<<sq;
        KnightAttack[sq] = shift_left(shift_2_up(mask))
                           | shift_right(shift_2_up(mask))
                           | shift_left(shift_2_down(mask))
                           | shift_right(shift_2_down(mask))
                           | shift_2_left(shift_up(mask))
                           | shift_2_right(shift_up(mask))
                           | shift_2_left(shift_down(mask))
                           | shift_2_right(shift_down(mask));
        KingAttackRook[sq] = mask << 1 | mask >> 1 | mask << 8 | mask >> 8;
        if(color(sq) == White) BBColorSquare[White] |= mask;
        else BBColorSquare[Black] |= mask;
    }

    for(Square s=A1; s<=H8; s++) {
        mask = Bitboard(1)<<s;
        KingAttack[s] = shift_left(mask) | shift_right(mask)
                        | shift_up(mask) | shift_down(mask)
                        | shift_up_left(mask) | shift_up_right(mask)
                        | shift_down_left(mask) | shift_down_right(mask);
        mask = KingAttack[s];
        KingAttack2[s] = shift_left(mask) | shift_right(mask)
                        | shift_up(mask) | shift_down(mask)
                        | shift_up_left(mask) | shift_up_right(mask)
                        | shift_down_left(mask) | shift_down_right(mask);
    }
    bb_attack[W] = bb_attack[B] = NULL;
    bb_attack[WP] = bb_attack[BP] = NULL;
    bb_attack[WN] = bb_attack[BN] = &bb_knight_attack;
    bb_attack[WB] = bb_attack[BB] = &bb_bishop_attack;
    bb_attack[WR] = bb_attack[BR] = &bb_rook_attack;
    bb_attack[WQ] = bb_attack[BQ] = &bb_queen_attack;
    bb_attack[WK] = bb_attack[BK] = NULL;
}

void init_magic_bb(){
    Square init_magic_bitpos64_db[64] = {
        Square(63), Square(0),  Square(58), Square(1),
        Square(59), Square(47), Square(53), Square(2),
        Square(60), Square(39), Square(48), Square(27),
        Square(54), Square(33), Square(42), Square(3),
        Square(61), Square(51), Square(37), Square(40),
        Square(49), Square(18), Square(28), Square(20),
        Square(55), Square(30), Square(34), Square(11),
        Square(43), Square(14), Square(22), Square(4),
        Square(62), Square(57), Square(46), Square(52),
        Square(38), Square(26), Square(32), Square(41),
        Square(50), Square(36), Square(17), Square(19),
        Square(29), Square(10), Square(13), Square(21),
        Square(56), Square(45), Square(25), Square(31),
        Square(35), Square(16), Square(9),  Square(12),
        Square(44), Square(24), Square(15), Square(8),
        Square(23),  Square(7), Square(6),  Square(5)
    };

    Bitboard* magic_bb_b_indices2[64] = {
        magic_bb_b_db+4992, magic_bb_b_db+2624, magic_bb_b_db+256,
        magic_bb_b_db+896,  magic_bb_b_db+1280, magic_bb_b_db+1664,
        magic_bb_b_db+4800, magic_bb_b_db+5120, magic_bb_b_db+2560,
        magic_bb_b_db+2656, magic_bb_b_db+288,  magic_bb_b_db+928,
        magic_bb_b_db+1312, magic_bb_b_db+1696, magic_bb_b_db+4832,
        magic_bb_b_db+4928, magic_bb_b_db+0,    magic_bb_b_db+128,
        magic_bb_b_db+320,  magic_bb_b_db+960,  magic_bb_b_db+1344,
        magic_bb_b_db+1728, magic_bb_b_db+2304, magic_bb_b_db+2432,
        magic_bb_b_db+32,   magic_bb_b_db+160,  magic_bb_b_db+448,
        magic_bb_b_db+2752, magic_bb_b_db+3776, magic_bb_b_db+1856,
        magic_bb_b_db+2336, magic_bb_b_db+2464, magic_bb_b_db+64,
        magic_bb_b_db+192,  magic_bb_b_db+576,  magic_bb_b_db+3264,
        magic_bb_b_db+4288, magic_bb_b_db+1984, magic_bb_b_db+2368,
        magic_bb_b_db+2496, magic_bb_b_db+96,   magic_bb_b_db+224,
        magic_bb_b_db+704,  magic_bb_b_db+1088, magic_bb_b_db+1472,
        magic_bb_b_db+2112, magic_bb_b_db+2400, magic_bb_b_db+2528,
        magic_bb_b_db+2592, magic_bb_b_db+2688, magic_bb_b_db+832,
        magic_bb_b_db+1216, magic_bb_b_db+1600, magic_bb_b_db+2240,
        magic_bb_b_db+4864, magic_bb_b_db+4960, magic_bb_b_db+5056,
        magic_bb_b_db+2720, magic_bb_b_db+864,  magic_bb_b_db+1248,
        magic_bb_b_db+1632, magic_bb_b_db+2272, magic_bb_b_db+4896,
        magic_bb_b_db+5184
    };

    Bitboard* magic_bb_r_indices2[64] = {
        magic_bb_r_db+86016, magic_bb_r_db+73728, magic_bb_r_db+36864,
        magic_bb_r_db+43008, magic_bb_r_db+47104, magic_bb_r_db+51200,
        magic_bb_r_db+77824, magic_bb_r_db+94208, magic_bb_r_db+69632,
        magic_bb_r_db+32768, magic_bb_r_db+38912, magic_bb_r_db+10240,
        magic_bb_r_db+14336, magic_bb_r_db+53248, magic_bb_r_db+57344,
        magic_bb_r_db+81920, magic_bb_r_db+24576, magic_bb_r_db+33792,
        magic_bb_r_db+6144,  magic_bb_r_db+11264, magic_bb_r_db+15360,
        magic_bb_r_db+18432, magic_bb_r_db+58368, magic_bb_r_db+61440,
        magic_bb_r_db+26624, magic_bb_r_db+4096,  magic_bb_r_db+7168,
        magic_bb_r_db+0,     magic_bb_r_db+2048,  magic_bb_r_db+19456,
        magic_bb_r_db+22528, magic_bb_r_db+63488, magic_bb_r_db+28672,
        magic_bb_r_db+5120,  magic_bb_r_db+8192,  magic_bb_r_db+1024,
        magic_bb_r_db+3072,  magic_bb_r_db+20480, magic_bb_r_db+23552,
        magic_bb_r_db+65536, magic_bb_r_db+30720, magic_bb_r_db+34816,
        magic_bb_r_db+9216,  magic_bb_r_db+12288, magic_bb_r_db+16384,
        magic_bb_r_db+21504, magic_bb_r_db+59392, magic_bb_r_db+67584,
        magic_bb_r_db+71680, magic_bb_r_db+35840, magic_bb_r_db+39936,
        magic_bb_r_db+13312, magic_bb_r_db+17408, magic_bb_r_db+54272,
        magic_bb_r_db+60416, magic_bb_r_db+83968, magic_bb_r_db+90112,
        magic_bb_r_db+75776, magic_bb_r_db+40960, magic_bb_r_db+45056,
        magic_bb_r_db+49152, magic_bb_r_db+55296, magic_bb_r_db+79872,
        magic_bb_r_db+98304
    };

    for(Square i=A1;i<=H8;i++){
        Square sq[64];
        int numSq=0;
        Bitboard temp=magic_bb_b_mask[i];
        while(temp){
            Bitboard bit=temp&-temp;
            sq[numSq++]=init_magic_bitpos64_db[
                    (bit*Bitboard(0x07EDD5E59A4E28C2ull))>>58];
            temp^=bit;
        }
        for(temp=0;temp<(((Bitboard)(1))<<numSq);temp++){
            Bitboard tempocc=init_magic_bb_occ(sq,numSq,temp);
            *(magic_bb_b_indices2[i]
              +(((tempocc)*magic_bb_b_magics[i])>>magic_bb_b_shift[i]))=
                init_magic_bb_b(i,tempocc);
        }
    }

    for(Square i=A1;i<=H8;i++){
        Square sq[64];
        int numSq=0;
        Bitboard temp=magic_bb_r_mask[i];
        while(temp){
            Bitboard bit=temp&-temp;
            sq[numSq++]=init_magic_bitpos64_db
                        [(bit*Bitboard(0x07EDD5E59A4E28C2ull))>>58];
            temp^=bit;
        }
        for(temp=0;temp<(((Bitboard)(1))<<numSq);temp++){
            Bitboard tempocc=init_magic_bb_occ(sq,numSq,temp);
            *(magic_bb_r_indices2[i]+
              (((tempocc)*magic_bb_r_magics[i])>>magic_bb_r_shift[i]))=
                init_magic_bb_r(i,tempocc);
        }
    }
}

Bitboard init_magic_bb_r(const Square sq, const Bitboard occ){
    Bitboard ret=0;
    Bitboard bit;
    Bitboard rowbits=(((Bitboard)0xFF)<<(8*(sq/8)));

    bit=(((Bitboard)(1))<<sq);
    do{
        bit<<=8;
        ret|=bit;
    }while(bit && !(bit&occ));
    bit=(((Bitboard)(1))<<sq);
    do{
        bit>>=8;
        ret|=bit;
    }while(bit && !(bit&occ));
    bit=(((Bitboard)(1))<<sq);
    do{
        bit<<=1;
        if(bit&rowbits) ret|=bit;
        else break;
    }while(!(bit&occ));
    bit=(((Bitboard)(1))<<sq);
    do{
        bit>>=1;
        if(bit&rowbits) ret|=bit;
        else break;
    }while(!(bit&occ));
    return ret;
}

Bitboard init_magic_bb_b(const Square sq, const Bitboard occ){
    Bitboard ret=0;
    Bitboard bit;
    Bitboard bit2;
    Bitboard rowbits=(((Bitboard)0xFF)<<(8*(sq/8)));

    bit=(((Bitboard)(1))<<sq);
    bit2=bit;
    do{
        bit<<=8-1;
        bit2>>=1;
        if(bit2&rowbits) ret|=bit;
        else break;
    }while(bit && !(bit&occ));
    bit=(((Bitboard)(1))<<sq);
    bit2=bit;
    do{
        bit<<=8+1;
        bit2<<=1;
        if(bit2&rowbits) ret|=bit;
        else break;
    }while(bit && !(bit&occ));
    bit=(((Bitboard)(1))<<sq);
    bit2=bit;
    do{
        bit>>=8-1;
        bit2<<=1;
        if(bit2&rowbits) ret|=bit;
        else break;
    }while(bit && !(bit&occ));
    bit=(((Bitboard)(1))<<sq);
    bit2=bit;
    do{
        bit>>=8+1;
        bit2>>=1;
        if(bit2&rowbits) ret|=bit;
        else break;
    }while(bit && !(bit&occ));
    return ret;
}

Bitboard init_magic_bb_occ(const Square* sq, const int numSq,
                           const Bitboard linocc){
    Bitboard ret=0;
    for(int i=0;i<numSq;i++)
        if(linocc&(((Bitboard)(1))<<i))
            ret|=(((Bitboard)(1))<<sq[i]);
    return ret;
}
