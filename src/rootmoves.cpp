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

#include <cstring>

#include "rootmoves.h"
#include "movegen.h"
#include "trans.h"
#include "gtb.h"
#include "engine.h"

int RootMoves::size;
int RootMoves::current;
RootMove RootMoves::rmove[300];
int RootMoves::multi_pv = 1;
Mutex RootMoves::bm_mutex;
bool RootMoves::bm_returned;
int RootMoves::sel_time;
Move RootMoves::ponder_move;

void RootMoves::init(){
    MutexInitialize(bm_mutex);
}

void RootMoves::new_search(const Thread& thread, const Move tt_move){
    MoveGenerator mg(thread,tt_move,0,0);
    current = size = 0;
    Move m;
    int eval;
    while((m = mg.next(thread,eval)) != NullMove){
        rmove[size].depth = Depth(0);
        rmove[size].eval = 0;
        rmove[size].flag = Exact;
        rmove[size].time = 0;
        rmove[size].pv[0] = NullMove;
        rmove[size++].move = m;
    }
    sel_time = 0;
    bm_returned = false;
    ponder_move = NullMove;
}

RootMove* RootMoves::get_next(const int alpha, const int beta,
                              const Depth depth){
    int a;

    for(int i=0; i<size; i++){
        if(i < multi_pv){
            if(multi_pv > 1) a = std::min(alpha,rmove[0].eval-Search::delta);
            else a = alpha;
        }
        else a = pv_alpha();

        if(rmove[i].depth < depth
           || (rmove[i].depth == depth && rmove[i].flag == Alpha
              && rmove[i].eval < beta)
           || (rmove[i].depth == depth && rmove[i].flag == Beta
             && rmove[i].eval > a)){
            current = i;
            return &rmove[i];
        }
    }
    return NULL;
}

bool RootMoves::first(){
    return current == 0;
}

bool RootMoves::pv(){
    return current < multi_pv;
}

RootMove* RootMoves::update(const SearchStack * ss, const int eval,
                            const int flag, const Depth depth){
    rmove[current].eval = eval;
    rmove[current].flag = flag;
    rmove[current].depth = depth;    
    memcpy(rmove[current].pv,ss->pv,(MAX_SEARCH_PLY+1)*sizeof(Move));
    RootMove r_tmp;

    for(; current>0; current--){
        if(rmove[current].depth == rmove[current-1].depth
           && rmove[current].eval > rmove[current-1].eval
           && rmove[current].flag != Beta){
            if(current == 1)
                sel_time = Engine::get_system_time() - Search::start_time;
            r_tmp = rmove[current-1];
            rmove[current-1] = rmove[current];
            rmove[current] = r_tmp;
        }
        else break;
    }

    for(; current<size-1; current++){
        if(rmove[current].depth == rmove[current+1].depth
           && rmove[current].eval < rmove[current+1].eval
           && rmove[current+1].flag != Beta){
            if(current == size-2)
                sel_time = Engine::get_system_time() - Search::start_time;
            r_tmp = rmove[current+1];
            rmove[current+1] = rmove[current];
            rmove[current] = r_tmp;
        }
        else break;
    }

    rmove[current].time = Engine::get_system_time() - Search::start_time;
    if(rmove[current].time == 0) rmove[current].time = 1;

    if(pv()) print_best_line();

    return &rmove[current];
}


void RootMoves::set_best_move(const Move move){
    RootMove tmp_root_move = rmove[0];
    for(current=0; current<size; current++){
        if(rmove[current].move == move){
            rmove[0] = rmove[current];
            rmove[current] = tmp_root_move;
            break;
        }
    }
}

RootMove* RootMoves::get(const int i){
    return &rmove[i];
}

int RootMoves::get_size(){
    return size;
}

void RootMoves::print_best_line(){
     #if __arm__
    if(Search::ids_depth < 10*Ply) return;
    #else
    if(Search::ids_depth < 5*Ply) return;
    #endif
    double time = Engine::get_system_time() - Search::start_time;
    if(time < 1) time = 1;
    Search::nps = int(Search::nodes/(time/1000.0));
    for(int i=0; i< multi_pv; i++){
        RootMove* rm = get(i);
        std::cout<<"info multipv "<<i+1;
        if(rm->eval > MATE_EVAL){
            if(Engine::pos.get_stm() == White)
                std::cout<<" score mate "<<((MATE - rm->eval+1)/2);
            else std::cout<<" score mate "<<((MATE - rm->eval+2)/2);
        }
        else if(rm->eval < -MATE_EVAL){
            if(Engine::pos.get_stm() == White)
                std::cout<<" score mate "<<((-MATE-rm->eval-2)/2);
            else std::cout<<" score mate "<<((-MATE-rm->eval-2)/2);
        }
        else std::cout<<" score cp "<<rm->eval;
        std::cout<<" depth "<<rm->depth/Ply<<" time "<<rm->time;
        std::cout<<" nodes "<<Search::nodes<<" nps "<<Search::nps;
        std::cout<<" hashfull "<<Trans::usage();
        std::cout<<" tbhits "<<GTB::hits;
        std::cout<<" pv "<<move_to_string(rm->move)<<" ";
        int index = 0;
        while(rm->pv[index] != NullMove){
            std::cout<<move_to_string(rm->pv[index])<<" ";
            index++;
        }
        std::cout<<std::endl;
    }
    if(get(0)->pv[0] != NullMove) ponder_move = get(0)->pv[0];
}

void RootMoves::print(){
    for(int i=0; i< size; i++){
        RootMove* rm = get(i);
        std::cout<<"info debug multipv "<<i+1;
        if(rm->eval > MATE_EVAL){
            if(Thread::thread[0]->pos.get_stm() == White)
                std::cout<<" score mate "<<((MATE - rm->eval+2)/2)<<" cp "<<rm->eval;
            else std::cout<<" score mate "<<((MATE - rm->eval+3)/2)<<" cp "<<rm->eval;
        }
        else if(rm->eval < -MATE_EVAL){
            if(Thread::thread[0]->pos.get_stm() == White)
                std::cout<<" score mate "<<((-MATE-rm->eval-1)/2)<<" cp "<<rm->eval;
            else std::cout<<" score mate "<<((-MATE-rm->eval-2)/2)<<" cp "<<rm->eval;
        }
        else std::cout<<" score cp "<<rm->eval;
        std::cout<<rm->eval<<" depth "<<rm->depth/Ply;
        std::cout<<" pv "<<move_to_string(rm->move);
        if(rm->flag == Alpha) std::cout<<" flag alpha";
        if(rm->flag == Beta) std::cout<<" flag beta";
        if(rm->flag == Exact) std::cout<<" flag exact";
        std::cout<<std::endl;
    }
}

void RootMoves::print_current_move(){
     #if __arm__
    if(Search::ids_depth < 15*Ply) return;
    #else
    if(Search::ids_depth < 15*Ply) return;
    #endif
    double time = Engine::get_system_time() - Search::start_time;
    if(time < 1) time = 1;
    Search::nps = int(Search::nodes/(time/1000.0));
    std::cout<<"info depth "<<Search::ids_depth/Ply<<" seldepth "<<Search::max_ply;
    std::cout<<" time "<<int(time)<<" nodes "<<Search::nodes;
    std::cout<<" nps "<<Search::nps;
    std::cout<<" hashfull "<<Trans::usage();
    std::cout<<" tbhits "<<GTB::hits<<std::endl;
    std::cout<<"info currmovenumber "<<current+1;
    std::cout<<" currmove "<<move_to_string(rmove[current].move)<<std::endl;
}

void RootMoves::print_best_move(){
    MutexLock(bm_mutex);
    if(!bm_returned){
        double time = Engine::get_system_time() - Search::start_time;
        if(time < 1) time = 1;
        Search::nps = int(Search::nodes/(time/1000));
        std::cout<<"info depth "<<(Search::ids_depth-Ply)/Ply;
        std::cout<<" seldepth "<<Search::max_ply;
        std::cout<<" time "<<int(time)<<" nodes "<<Search::nodes;
        std::cout<<" nps "<<Search::nps;
        std::cout<<" hashfull "<<Trans::usage();
        std::cout<<" tbhits "<<GTB::hits<<std::endl;
        std::cout<<"bestmove "<<move_to_string(get(0)->move);
        if(Engine::ponder && ponder_move != NullMove)
            std::cout<<" ponder "<<move_to_string(ponder_move);
        std::cout<<std::endl;
        bm_returned = true;
    }
    MutexUnlock(bm_mutex);
}

int RootMoves::move_number(){
    return current + 1;
}

int RootMoves::pv_alpha(){
    return rmove[multi_pv-1].eval;
}

void RootMoves::update_mpv_in_tt(Thread & thread, const Depth depth){
    for(int i=0; i<multi_pv; i++) update_pv_in_tt(thread,rmove[i],depth);
}

void RootMoves::update_pv_in_tt(Thread & thread, const RootMove & rm,
                                Depth depth){
    int ply = 0;
    thread.pos.move_do(rm.move);
    depth -= Ply;
    while(rm.pv[ply] != NullMove){
        depth = Trans::update_pv(thread, rm.pv[ply], rm.eval, rm.flag, ply,
                                 depth);
        thread.pos.move_do(rm.pv[ply]);
        depth -= Ply;
        ply++;
    }
    while(ply >= 0){
        thread.pos.move_undo();
        ply--;
    }
}
