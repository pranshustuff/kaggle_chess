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
/*   This code based on based on Toga II code.                             *
 ***************************************************************************/

#include <cstdlib>

#include "movegen.h"
#include "thread.h"

int MoveGenerator::state[20];

int MoveGenerator::GenS;
int MoveGenerator::GenQS;
int MoveGenerator::GenEvasionS;
int MoveGenerator::GenEvasionQS;
int MoveGenerator::GenCheckQS;

void MoveGenerator::init(){
    int s = 0;

    GenS = s;
    state[s++] = TRANS;
    state[s++] = GOOD_CAPTURE;
    state[s++] = KILLER;
    state[s++] = QUIET_H;
    state[s++] = BAD_CAPTURE;
    state[s++] = END;

    GenQS = s;
    state[s++] = TRANS;
    state[s++] = CAPTURE_QS;
    state[s++] = END;

    GenCheckQS = s;
    state[s++] = TRANS;
    state[s++] = CAPTURE_QS;
    state[s++] = CHECK_QS;
    state[s++] = END;

    GenEvasionS = s;
    state[s++] = TRANS;
    state[s++] = EVASION;
    state[s++] = BAD_EVASION_CAPTURE;
    state[s++] = END;

    GenEvasionQS = s;
    state[s++] = TRANS;
    state[s++] = EVASION_QS;
    state[s++] = END;
}

MoveGenerator::MoveGenerator(const Thread& thread, const Move tt_move,
                             const int depth){
    this->ttMove = tt_move;

    if(thread.pos.is_in_check()){
        index = GenEvasionQS;
        thread.pos.gen_evasions(ml);
        if(ml.get_size() == 0){
            index++;
            return;
        }
        ml.sort();
    }
    else if(depth < 0) index = GenQS;
    else index = GenCheckQS;
}

MoveGenerator::MoveGenerator(const Thread& thread, const Move ttMove,
                             const int ply, const int /* depth*/){
    this->ttMove = ttMove;
    this->ply = ply;

    if(thread.pos.is_in_check()){
        index = GenEvasionS;
        thread.pos.gen_evasions(ml);
        if(ml.get_size() == 0){
            index+=3;
            return;
        }
        ml.sort();
    }
    else index = GenS;

    kindex = -1;
}

Move MoveGenerator::next(const Thread& thread, int & eval){
    Move move;
    eval = HistoryMax;
    while(state[index] != END){
        switch(state[index]){
            case TRANS:{
                if(ttMove != NullMove){
                    #ifndef NDEBUG
                    if(!thread.pos.move_gen_legal(ttMove)){
                        std::cerr<<"Wrong ttMove!"<<std::endl;
                        exit(1);
                    }
                    #endif
                    index ++;
                    return ttMove;
                }
                break;
            }
            case GOOD_CAPTURE:{
                if(ml.get_size() == 0){
                    thread.pos.gen_capture(ml);
                    if(ml.get_size() == 0) break;
                    ml.sort();
                }
                while(ml.get_next(move)){
                    if(move == ttMove || !thread.pos.move_gen_legal(move))
                        continue;
                    if(thread.pos.see(move) < SSE_GOOD){
                        bad.add(move);
                        continue;
                    }
                    return move;
                }
                ml.clear();
                break;
            }
            case KILLER:{
                if(kindex == 2) break;
                if(kindex == -1){
                    killer[0] = thread.ss[ply].killer[0];
                    killer[1] = thread.ss[ply].killer[1];
                    kindex = 0;
                }
                while(kindex < 2){
                    if(killer[kindex] == ttMove
                       || !thread.pos.move_legal(killer[kindex])
                       || !thread.pos.move_gen_legal(killer[kindex])){
                        kindex ++;
                        continue;
                    }
                    return killer[kindex++];
                }
                break;
            }
            case BAD_CAPTURE:{
                while(bad.get_next(move)){
                    if(move != ttMove && move != killer[0] && move != killer[1]
                       && thread.pos.move_gen_legal(move))
                        return move;
                }
                break;
            }
            case BAD_EVASION_CAPTURE:{
                while(bad.get_next(move)){
                    if(move != ttMove && thread.pos.move_gen_legal(move))
                        return move;
                }
                break;
            }
            case QUIET_H:{
                if(ml.get_size() == 0){
                    thread.pos.gen_quiet(ml);
                    if(ml.get_size() == 0) break;
                    ml.sort();
                }
                while(ml.get_next(move)){
                    if(move != ttMove && move != killer[0] && move != killer[1]
                       && thread.pos.move_gen_legal(move)){
                        eval = Search::eval_history_move(thread,move);
                        return move;
                    }
                }
                ml.clear();
                break;
            }
            case EVASION:{
                while(ml.get_next(move)){
                    if(move == ttMove || !thread.pos.move_gen_legal(move))
                        continue;
                    if(thread.pos.see(move) < SSE_GOOD){
                        bad.add(move);
                        continue;
                    }
                    return move;
                }
                ml.clear();
                break;
            }
            case CAPTURE_QS:{
                if(ml.get_size() == 0){
                    thread.pos.gen_capture(ml);
                    if(ml.get_size() == 0) break;
                    ml.sort();
                }
                while(ml.get_next(move)){
                    if(move == ttMove ||!thread.pos.move_gen_legal(move)
                        || thread.pos.see(move) < SSE_GOOD)
                        continue;
                    return move;
                }
                ml.clear();
                break;
            }
            case CHECK_QS:{
                if(ml.get_size() == 0){
                    thread.pos.gen_quiet_checks(ml);
                    if(ml.get_size() == 0) break;
                }
                while(ml.get_next(move)){
                    if(move == ttMove || !thread.pos.move_gen_legal(move)
                        || thread.pos.see(move) < SSE_GOOD)
                        continue;
                    return move;
                }
                ml.clear();
                break;
            }
            case EVASION_QS:{
                while(ml.get_next(move)){
                    if(move != ttMove && thread.pos.move_gen_legal(move))
                        return move;
                }
                ml.clear();
                break;
            }
            default:{
                std::cerr<<"State of move generator is wrong!"<<index<<std::endl;
                exit(1);
            }
        }
        index++;
    }
    return NullMove;
}
