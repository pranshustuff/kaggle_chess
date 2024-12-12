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

#include <iostream>
#include <cstring>
#include <limits>
#include <cmath>

#include "search.h"
#include "trans.h"
#include "book.h"
#include "engine.h"
#include "gtb.h"

int Search::nps;
unsigned long long Search::nodes;
int Search::max_ply;
bool Search::infinite;
unsigned long long Search::max_nodes;
Depth Search::max_depth;
int Search::start_time;
Depth Search::ids_depth;
int Search::end_time;
bool Search::egtb_root;
Depth Search::egtb_depth;
bool Search::search_stop;
int Search::stop_nodes;
int Search::max_stop_nodes;
int Search::delta = 100;
bool Search::ponder;
Mutex Search::signal_mutex;
SplitPoint Search::sp;
bool Search::split_point_available;

int16_t Search::history[14][64][64];
int16_t Search::his_tot[14][64][64];
int16_t Search::his_hit[14][64][64];
int16_t Search::his_diff[14][64];
int16_t Search::his_eval_array[16*Ply];
int8_t Search::reduction_array[2][MAX_SEARCH_DEPTH][64];
int16_t Search::futility_margin_array[7*Ply][64];
int16_t Search::razor_margin_array[4*Ply];
int16_t Search::futility_move_nb_array[16*Ply];

Key Search::E_Key;

const int Search::iid_depth[2] = {5*Ply,8*Ply};
const int Search::singular_ext_depth[2] = {6*Ply,8*Ply};


void Search::init(){
    E_Key = myrand64();
    MutexInitialize(signal_mutex);
    for(int depth=0; depth<MAX_SEARCH_DEPTH; depth++){
        for(int mnb=0; mnb<64; mnb++){
            reduction_array[NodePVS][depth][mnb] =
                    Ply*(log(double(depth/Ply)) * log(double(mnb)) / 3.0);
            reduction_array[NodeMWS][depth][mnb] =
                    Ply*(0.33 + log(double(depth/Ply)) * log(double(mnb)) / 2.25);
        }
    }

    for (int depth = 0; depth < 7*Ply; depth++){
        for (int mnb = 0; mnb < 64; mnb++){
            if(depth > 0)
                futility_margin_array[depth][mnb] =
                        57*(log(depth*depth/2.0)/log(2.0)+1)-1*mnb + 23;
            else
                futility_margin_array[depth][mnb] =
                        57*(log(1.1)/log(2.0)+1)-1*mnb + 23;
            if(futility_margin_array[depth][mnb] < 0)
                futility_margin_array[depth][mnb] = 0;
        }
    }

    for (int depth = 0; depth < 16*Ply; depth++){
        futility_move_nb_array[depth] = int(3 + 0.25 * pow(depth, 2.0));
    }


    for(int depth=0; depth<16*Ply; depth ++){
        his_eval_array[depth] = (HistoryMax*0.7)/log((depth/Ply)*(depth/Ply)+2);
    }

    for(int depth=0; depth<4*Ply; depth ++){
        razor_margin_array[depth] = 256 + 8 * (depth / Ply);
    }

    reset();
}

template<NodeType node>
Depth Search::reduction(const Depth depth,const int mn){
    return (Depth)reduction_array[node]
            [std::min(int(depth),MAX_SEARCH_DEPTH-1)]
            [std::min(mn,63)];
}

inline int Search::futility_margin(const Depth depth, const int mn){
    return (Depth)futility_margin_array
            [std::max(int(depth),0)]
            [std::min(mn, 63)];
}

inline int Search::razor_margin(const Depth depth){
    return razor_margin_array[depth];
}

inline int Search::futility_move_nb(const Depth depth) {
    return futility_move_nb_array[depth];
}

inline int Search::his_eval(const Depth depth) {
    return his_eval_array[depth];
}

void Search::reset(){ // PVT + memcopy !!!
    for(Piece p = Piece(PAWN); p<=BK; p++){
        for(Square sq_f = A1; sq_f<=H8; sq_f++){
            for(Square sq_t = A1; sq_t<=H8; sq_t++){
                history[p][sq_f][sq_t] = 0;
                his_hit[p][sq_f][sq_t] = 1;
                his_tot[p][sq_f][sq_t] = 1;
            }
            his_diff[p][sq_f] = 0;
        }
    }
}

inline void Search::update_his_diff(const SearchStack* ss,
                                    const Thread& thread){
    Move move = (ss-1)->current_move;
    if(move != NullMove && !is_tactical(move)){
        const Square to = move_to(move);
        const Piece p = thread.pos.get_piece(to);
        int16_t delta = 2*(-((ss-1)->static_eval+ ss->static_eval));
        if(delta >= his_diff[p][to]) his_diff[p][to] = delta;
        else his_diff[p][to]--;
    }
}

inline int Search::get_his_diff(const Thread & thread, const Move move){
    return his_diff[thread.pos.get_piece(move_from(move))][move_to(move)] / 2;
}

void Search::start(const Position& pos, const bool infinite,
                   const Depth max_depth, const unsigned long long max_nodes){
    end_time = Engine::get_search_time();
    start_time = Engine::get_system_time();
    Search::infinite = infinite;
    Search::max_nodes = max_nodes;
    Search::max_depth = max_depth;
    search_stop = false;
    Thread::search_start(pos);
}

void Search::ids(Thread & thread){
    Depth depth;
    int window, alpha, beta, eval;
    Move tt_move = NullMove;
    max_ply = 0;
    ids_depth = Ply;
    eval = 0;
    nodes = 0;
    GTB::hits = 0;
    stop_nodes = 0;
    max_stop_nodes = 10;

    TransRecord * tr;
    TransData td;
    if(Trans::find(&tr,thread.pos.get_key(),td,0)){
        tt_move = td.move;
        if(tt_move != NullMove && td.flag == Exact){
            eval = td.eval;
            ids_depth = Depth(td.depth) - Ply;
        }
    }

    RootMoves::new_search(thread,tt_move);
    if(!infinite && max_depth == 0 && max_nodes == 0){
        if((Book::is_loaded() && Book::find_move(thread.pos))
           || RootMoves::get_size() == 1){
            while(true){
                MutexLock(signal_mutex);
                if(!Search::ponder || Search::search_stop){
                    MutexUnlock(signal_mutex);
                    break;
                }
                MutexUnlock(signal_mutex);
            }
            RootMoves::print_best_move();
            return;
        }
    }

    if(ids_depth == 1){
        reset();
        Trans::new_age();
    }

    int piece_num = thread.pos.num_pieces();
    egtb_root = false;
    if(piece_num <= 5 && GTB::isLoaded())
        egtb_root = true;

    split_point_available = false;
    if(Thread::threads_num > 1) split_point_available = true;

    if(max_depth == 0) depth = MAX_SEARCH_DEPTH;
    else depth = max_depth;

    thread.ss[0].init<NodePVS>(thread.pos.get_key());
    thread.ss[0].static_eval = thread.pos.eval<false>(thread,
                                                      thread.ss[0].eval_margin);

    for(;ids_depth<=depth && !should_stop(thread); ids_depth+=Ply){
        window = 20;
        alpha = eval - window;
        beta = eval + window;
        egtb_depth = Depth(2 * ids_depth / 3);
        while(true){
            eval = rs(thread.ss,thread,alpha,beta,ids_depth);
            if(should_stop(thread) || (eval > alpha && eval < beta)) break;
            window += window;
            alpha = eval-window;
            beta = eval + window;
        }
        RootMoves::update_mpv_in_tt(thread,ids_depth);
    }
    if(!infinite){
        while(true){
            MutexLock(signal_mutex);
            if(!ponder || search_stop){
                MutexUnlock(signal_mutex);
                break;
            }
            MutexUnlock(signal_mutex);
        }
        RootMoves::print_best_move();
    }
}

int Search::rs(SearchStack* ss, Thread & thread, int alpha, int beta,
               const Depth depth){
    RootMove* r_move;
    Depth search_depth;
    int eval, move_nb, a;
    bool dangerous;

    nodes++;

    while(alpha < beta &&
          (r_move = RootMoves::get_next(alpha, beta, depth)) != NULL
          && !should_stop(thread)){
        max_stop_nodes = 0;
        RootMoves::print_current_move();
        move_nb = RootMoves::move_number();

        search_depth = new_depth<NodePVS>(thread, r_move->move, false,
                                          dangerous, depth);

        ss->current_move = r_move->move;
        thread.pos.move_do(r_move->move);

        if(egtb_root && GTB::gtb_probe_hard(thread.pos,eval,0)){
            a = std::min(alpha, RootMoves::get(0)->eval - delta);
            eval = -eval;
            (ss+1)->init<NodePVS>(thread.pos.get_key());
        }
        else if(RootMoves::pv()){
            if(RootMoves::multi_pv == 1) a = alpha;
            else a = std::min(alpha, RootMoves::get(0)->eval - delta);
            eval = -search<NodePVS>(ss+1,thread,-beta,-a,1,search_depth);
        }
        else{
            if(!thread.pos.is_in_check() && !dangerous && depth > 2
               && !is_tactical(r_move->move) && !is_castle(r_move->move))
                ss->reduction = reduction<NodePVS>(depth,move_nb);
            else ss->reduction = Depth(0);

            a = RootMoves::pv_alpha();
            eval = a + 1;
            if(ss->reduction > 0){
                eval = -search<NodeMWS>(ss+1,thread,-a-1,-a,1,
                                        search_depth-ss->reduction);
                if(eval > a && ss->reduction > Ply)
                    eval = - search<NodeMWS>(ss+1,thread,-a-1,-a,1,
                                             search_depth-Ply);
            }
            if(eval > a){
                eval = -search<NodeMWS>(ss+1,thread,-a-1,-a,1,search_depth);
                if(eval > a)
                    eval = -search<NodePVS>(ss+1,thread,-beta,-a,1,
                                            search_depth);
            }
        }
        thread.pos.move_undo();
        if(should_stop(thread)) break;

        if(eval > a){
            if(eval > alpha) alpha = eval;
            if(eval >= beta) r_move = RootMoves::update(ss+1,eval,Alpha,depth);
             else r_move = RootMoves::update(ss+1,eval,Exact,depth);
        }
        else r_move = RootMoves::update(ss+1,a,Beta,depth);
    }
    ss->best_move = RootMoves::get(0)->move;
    return RootMoves::get(0)->eval;
}

template <NodeType node>
int Search::search(SearchStack* ss, Thread& thread, int alpha, int beta,
                   const int ply, const Depth depth){
    const int old_alpha = alpha;
    int margin, eval;
    Move move, tt_move;
    Depth search_depth;

    nodes ++;
    stop_nodes++;
    Key key = thread.pos.get_key();
    if(node == NodeMWS && ss->eMove != NullMove) key ^= E_Key;
    ss->init<node>(key);

    if(thread.pos.draw()) return DRAW;

    if(ply > max_ply) max_ply = ply;

    // Transposition table
    TransRecord * tr;
    TransData td;
    const bool tt_found = Trans::find(&tr,key,td,ply);

    // Endgame databases
    int piece_num = thread.pos.num_pieces();
    if(piece_num <= 5){
        if(GTB::isLoaded()){
            if(alpha - ply <= -MATE_EVAL || beta + ply >= MATE_EVAL){
                if(tt_found && td.flag == Egtb && mate_value(td.eval)){
                    if(depth > td.depth)
                        Trans::write(tr,key,depth,td.eval,td.s_eval,
                                     td.eval_margin,Egtb,NullMove,ply);
                    else Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
                if((node == NodePVS && depth >= egtb_depth / 2)
                    || (node == NodeMWS && depth >= egtb_depth)){
                    if(GTB::gtb_probe_hard(thread.pos,eval,ply)){
                        Trans::write(tr,key,depth,eval,eval,0,Egtb,NullMove,
                                     ply);
                        return eval;
                    }
                }
                else if(node == NodePVS || depth >= egtb_depth / 4){
                    if(GTB::gtb_probe_soft(thread.pos,eval,ply)){
                        Trans::write(tr,key,depth,eval,eval,0,Egtb,NullMove,ply);
                        return eval;
                    }
                }
            }
            else{
                if(tt_found && td.flag == Egtb){
                    if(depth > td.depth)
                        Trans::write(tr,key,depth,td.eval,td.s_eval,
                                     td.eval_margin,Egtb,NullMove,ply);
                    else Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
                if((node == NodePVS && depth >= egtb_depth / 2)
                    || (node == NodeMWS && depth >= egtb_depth)){
                    if(GTB::gtb_probe_WDL_hard(thread.pos,eval,ply)){
                        Trans::write(tr,key,depth,eval,eval,0,Egtb,NullMove,
                                     ply);
                        return eval;
                    }
                }else if(node == NodePVS || depth >= egtb_depth / 4){
                    if(GTB::gtb_probe_WDL_soft(thread.pos,eval,ply)){
                        Trans::write(tr,key,depth,eval,eval,0,Egtb,NullMove,ply);
                        return eval;
                    }
                }
            }
            if(piece_num <= 4 && thread.pos.eg_draw()){
                Trans::write(tr,key,depth,DRAW,DRAW,0,Egtb,NullMove,ply);
                return DRAW;
            }
        }
        else if(piece_num <= 4){
            if(tt_found && td.flag == Egtb){
                Trans::refresh(tr,key,td.age);
                return td.eval;
            }
            else if(thread.pos.eg_draw()){
                Trans::write(tr,key,depth,DRAW,DRAW,0,Egtb,NullMove,ply);
                return DRAW;
            }
        }
    }

    // Transposition table
    if(tt_found){
        tt_move = (Move)td.move;
        if(node == NodeMWS){
            if(td.depth < depth){
                if(td.eval < -MATE + MAX_SEARCH_PLY
                   && (td.flag == Beta || td.flag == Exact)){
                    td.depth = depth;
                    td.flag = Beta;
                }
                if(td.eval > MATE - MAX_SEARCH_PLY
                   && (td.flag == Alpha || td.flag == Exact)){
                    td.depth = depth;
                    td.flag = Alpha;
                }
            }
            if(td.depth >= depth){
                if(td.flag == Exact){
                    ss->best_move = td.move;
                    Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
                else if(td.flag == Alpha && td.eval > alpha){
                    ss->best_move = td.move;
                    Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
                else if(td.flag == Beta && td.eval < beta) {
                    ss->best_move = td.move;
                    Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
            }
        }
    }
    else tt_move = NullMove;

    if(depth <= 0) return qs<node>(ss,thread,alpha,beta,ply,Depth(0));

    bool is_check = thread.pos.is_in_check();

    // Mate distance pruning
    if(!is_check) eval = - MATE + (ply+2);
    else eval = - MATE + ply;
    if(alpha < eval) return eval;

    eval = MATE - ply;
    if(beta > eval) return eval;

    // Reevaluation
    int re_eval = - MATE;
    if(tt_found){
        ss->static_eval = td.s_eval;
        ss->eval_margin = td.eval_margin;
        if(node == NodeMWS){
            re_eval = ss->static_eval;
            if(td.flag == Exact) re_eval = td.eval;
            else if(td.flag == Alpha){
                if(td.eval > re_eval) re_eval = td.eval;
            }
            else if(td.flag == Beta) {
                if(td.eval < re_eval) re_eval = td.eval;
            }
        }
    }
    else{
        ss->static_eval = thread.pos.eval<false>(thread,ss->eval_margin);
        if(node == NodeMWS) re_eval = ss->static_eval;
    }

    update_his_diff(ss,thread);

    if(ply == MAX_SEARCH_PLY - 1) return ss->static_eval;

    // Razoring
    if(node == NodeMWS && tt_move == NullMove && depth < 4*Ply && !is_check
       && !mate_value(beta) && !thread.pos.pawn_7th()){
        margin = razor_margin(depth) + ss->eval_margin;
        if(re_eval < beta - margin){
            int rbeta = beta - margin;
            eval = qs<NodeMWS>(ss,thread,rbeta-1,rbeta,ply,Depth(0));
            if(eval < rbeta) return eval;
        }
    }

    // Static null move pruning
    if(node == NodeMWS && !ss->skip_null_move && depth < 4*Ply && !is_check
       && !mate_value(beta) && thread.pos.doNullMove()){
        margin = futility_margin(depth,0);
        if(re_eval >= beta + margin) return re_eval - margin;
    }

    // Null move pruning
    if(node == NodeMWS && !ss->skip_null_move && !is_check && depth >= 2*Ply
       && !mate_value(beta) && thread.pos.doNullMove() && re_eval >= beta){

        int R = (3 + (depth > 4*Ply ? depth / (4*Ply) : 0)) * Ply;
        if(re_eval - beta > PAWN_VALUE) R += Ply;

        ss->current_move = NullMove;
        thread.pos.move_do(NullMove);
        ss->skip_null_move = true;
        eval = -search<NodeMWS>(ss+1,thread,-beta,-alpha,ply+1,Depth(depth-R));
        ss->skip_null_move = false;
        thread.pos.move_undo();

        // Verification
        if(eval >= beta){
            if(eval >= MATE - ply) eval = beta; // Do not return mate
            if(depth > 5 * Ply){
                ss->skip_null_move = true;
                eval = search<NodeMWS>(ss,thread,alpha,beta,ply,Depth(depth - 5*Ply));
                ss->skip_null_move = false;
            }
        }

        if(eval >= beta){
            if(should_stop(thread)) return DRAW;
            Trans::write(tr,key,depth,eval,ss->static_eval,ss->eval_margin,
                         Alpha,ss->best_move,ply);
            return eval;
        }
    }

    // Internal iterative deepening
    if(depth >= iid_depth[node] && tt_move == NullMove
       && (node == NodePVS || (!is_check && ss->static_eval >= beta - 128))) {
        if(node == NodePVS) search_depth = depth-2*Ply;
        else search_depth = depth/2;
        ss->skip_null_move = true;
        eval = search<node>(ss,thread,alpha,beta,ply,search_depth);
        ss->skip_null_move = false;
        tt_move = ss->best_move;
    }

    MoveGenerator mg(thread,tt_move,ply,depth);

    bool single_replay = false;

    if(is_check){
        if(mg.size() == 0) return -MATE + ply;
        if(mg.size() == 1) single_replay = true;
    }

    int move_nb = 0, move_eval, h_eval, f_eval, pv_size;
    Move history_move[256];
    bool dangerous;

    if(node == NodeMWS){
        // Futitlity evalaution
        h_eval = 0;
        if(depth < 16*Ply) h_eval = his_eval(depth);
        f_eval = ss->static_eval + ss->eval_margin;
    }
    else
        pv_size = MAX_SEARCH_PLY+1-ply;

    // Searching of moves
    while((move = mg.next(thread,move_eval)) != NullMove){
        // Extensions
        search_depth = new_depth<node>(thread, move, single_replay, dangerous,
                                       depth);

        if(node == NodeMWS && move == ss->eMove) continue;
        history_move[move_nb ++] = move;

        // Singular extensions (condition remove from loop !!!)
        if(depth >= singular_ext_depth[node] && ss->eMove == NullMove
           && move == tt_move && search_depth < depth
           && (td.flag == Exact || td.flag == Alpha)
           && td.depth >= depth-3*Ply && !mate_value(td.eval)){
            const SearchStack tmp_ss = *ss;
            ss->skip_null_move = true;
            ss->eMove = move;
            eval = search<NodeMWS>(ss,thread,td.eval-65,td.eval-64,ply,depth/2);
            *ss = tmp_ss;
            if(eval < td.eval - 64) search_depth = depth;
        }

        ss->reduction = Depth(0);

        if(!is_check && !dangerous && !is_tactical(move)){

            // Reduction
            if(depth > 2*Ply && !is_castle(move)
                && move != ss->killer[0] && move != ss->killer[1])
                ss->reduction = reduction<node>(depth,move_nb);

            // Futility pruning (Move count based pruning)
            if(node == NodeMWS && depth < 16*Ply
               && move_nb > futility_move_nb(depth) && move_eval < h_eval)
                continue;

            // Futility pruning (Value based pruning)
            if(node == NodeMWS && move != tt_move
               && search_depth - ss->reduction < 7*Ply
               && f_eval + futility_margin(Depth(search_depth - ss->reduction),
                                           move_nb)
                  + get_his_diff(thread,move) < alpha)
                continue;

            // see pruning
            if(node == NodeMWS && search_depth - ss->reduction < 2*Ply
               && thread.pos.see(move) < SSE_GOOD)
                continue;
        }

        ss->current_move = move;
        thread.pos.move_do(move);

        // Recursive search
        if(node == NodePVS && move_nb == 1)
            eval = - search<NodePVS>(ss+1, thread, -beta, -alpha, ply+1,
                                     search_depth);
        else{
            eval = alpha + 1;
            if(ss->reduction > 0){
                eval = -search<NodeMWS>(ss+1,thread,-alpha-1,-alpha,ply+1,
                                        Depth(search_depth-ss->reduction));
                if(eval > alpha && ss->reduction > Ply)
                    eval = - search<NodeMWS>(ss+1,thread,-alpha-1,-alpha,ply+1,
                                             search_depth-Ply);
            }
            if(eval > alpha){
                eval = -search<NodeMWS>(ss+1,thread,-alpha-1,-alpha,ply+1,
                                        search_depth);
                if(node == NodePVS && eval > alpha)
                    eval = -search<NodePVS>(ss+1,thread,-beta,-alpha,ply+1,
                                            search_depth);
            }
        }

        thread.pos.move_undo();
        if(should_stop(thread)) return DRAW;

        if(eval > alpha){
            ss->best_move = move;
            alpha = eval;
            if(node == NodePVS){
                ss->pv[0] = move;
                memcpy(ss->pv+1,(ss+1)->pv,(pv_size-1)*sizeof(Move));
            }
            if(alpha >= beta) break;
        }

        // Parallel search
        if(split_point_available && thread.id == 0 && depth > 4 * Ply
           && !single_replay && !mate_value(beta)
           && split_point<node>(alpha, beta, ply, depth, is_check, move_nb,
                                 history_move, &mg, thread))
            break;
    }

    if((move_nb == 0 && (node == NodePVS || ss->eMove == NullMove))
        || should_stop(thread))
        return DRAW;

/*    if(is_check && ply >= ids_depth && depth <= 2 && alpha > DRAW
       && thread.pos.perpetual_check())
        alpha = DRAW;*/

    // Transpostion table and history hevristics
    if(alpha >= beta){
        if(!is_tactical(ss->best_move))
            move_good(thread,ss->best_move,history_move,move_nb,ply,depth);
        Trans::write(tr,key,depth,alpha,ss->static_eval,ss->eval_margin,Alpha,
                     ss->best_move,ply);
    }
    else if(alpha <= old_alpha)
        Trans::write(tr,key,depth,alpha,ss->static_eval,ss->eval_margin,Beta,
                      tt_move,ply);
    else
        Trans::write(tr,key,depth,alpha,ss->static_eval,ss->eval_margin,Exact,
                      ss->best_move,ply);

    return alpha;
}

template <NodeType node>
void Search::smp_search(Thread & thread, SplitPoint& sp){
    Depth search_depth;
    int eval, move_nb, alpha, move_eval, h_eval, f_eval, pv_size;
    Move move;
    bool dangerous;

    const bool single_replay = false;
    const bool is_check = sp.is_check;
    const Depth depth = sp.depth;
    const int ply = sp.ply;
    SearchStack * ss = &thread.ss[ply];
    SearchStack * ss0 = &Thread::thread[0]->ss[ply];
    const int beta = sp.beta;

    if(node == NodeMWS){
        h_eval = 0;
        if(depth < 16*Ply) h_eval = his_eval(depth);
        f_eval = ss->static_eval + ss->eval_margin;
    }
    else
        pv_size = MAX_SEARCH_PLY+1-ply;

    MutexLock(sp.mutex);
    while((move = sp.mg->next(thread,move_eval)) != NullMove){
        ss->current_move = sp.history_move[sp.move_nb ++] = move;
        alpha = sp.alpha;
        move_nb = sp.move_nb;
        MutexUnlock(sp.mutex);

        search_depth = new_depth<node>(thread, move, single_replay, dangerous,
                                       depth);
        ss->reduction = Depth(0);

        if(!is_check && !dangerous && !is_tactical(move)){

            // Reduction
            if(depth > 2*Ply && !is_castle(move)
                && move != thread.ss[ply].killer[0]
                && move != thread.ss[ply].killer[1])
                ss->reduction = reduction<node>(depth,move_nb);

            // Futility pruning (Move count based pruning)
            if(node == NodeMWS && depth < 16*Ply
               && move_nb > futility_move_nb(depth) && move_eval < h_eval){
                MutexLock(sp.mutex);
                continue;
            }

            // Futility pruning (Value based pruning)
            if(node == NodeMWS && search_depth - ss->reduction < 7*Ply
               && f_eval + futility_margin(search_depth-ss->reduction,move_nb)
                         + get_his_diff(thread,move) < alpha){
                MutexLock(sp.mutex);
                continue;
            }

            // see pruning
            if(node == NodeMWS && search_depth - ss->reduction < 2*Ply
               && thread.pos.see(move) < SSE_GOOD){
                MutexLock(sp.mutex);
                continue;
            }
        }

        thread.pos.move_do(move);

        eval = alpha + 1;
        if(ss->reduction > 0){
            eval = -search<NodeMWS>(ss+1,thread,-alpha-1,-alpha,ply+1,
                                    search_depth-ss->reduction);
            if(eval > alpha && ss->reduction > Ply)
                eval = -search<NodeMWS>(ss+1,thread,-alpha-1,-alpha,ply+1,
                                        search_depth-Ply);
        }
        if(eval > alpha){
            eval = -search<NodeMWS>(ss+1,thread,-alpha-1,-alpha,ply+1,
                                    search_depth);
            if(node == NodePVS && eval > alpha)
                eval = -search<NodePVS>(ss+1,thread,-beta, -alpha, ply+1,
                                        search_depth);
        }

        thread.pos.move_undo();

        if(should_stop(thread)) return;

        MutexLock(sp.mutex);
        if(eval > alpha){
            ss0->best_move = ss0->pv[0] = move;
            if(node == NodePVS)
                memcpy(ss0->pv+1,(ss+1)->pv,(pv_size-1)*sizeof(Move));
            sp.alpha = eval;
            break;
        }
    }
    MutexUnlock(sp.mutex);
}

template <NodeType node>
int Search::qs(SearchStack* ss, Thread & thread, int alpha, int beta,
               const int ply, Depth depth){
    int eval, re_eval, f_eval, pv_size;
    const int old_alpha = alpha;
    bool is_check;
    Move move;

    Key key = thread.pos.get_key();
    nodes ++;
    stop_nodes++;
    ss->init<node>(key);

    if(ply > max_ply) max_ply = ply;

    if(thread.pos.draw()) return DRAW;

    int piece_num = thread.pos.num_pieces();

    TransRecord * tr;
    TransData td;
    Move tt_move = NullMove;
    const bool tt_found = Trans::find(&tr,key,td,ply);

    // Endgame databases
    if(piece_num <= 5){
        if(GTB::isLoaded()){
            if(alpha - ply <= -MATE_EVAL || beta + ply >= MATE_EVAL){
                if(tt_found && td.flag == Egtb && mate_value(td.eval)){
                    Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
                if(node == NodePVS && GTB::gtb_probe_soft(thread.pos,eval,ply))
                    return eval;
            }
            else {
                if(tt_found && td.flag == Egtb){
                    Trans::refresh(tr,key,td.age);
                    return td.eval;
                }
                if(node == NodePVS && GTB::gtb_probe_WDL_soft(thread.pos,eval,ply))
                    return eval;
            }
            if(piece_num <= 4 && thread.pos.eg_eval(eval)){
                Trans::write(tr,key,depth,eval,eval,0,Exact,NullMove,ply);
                return eval;
            }
        }
        else if(piece_num <= 4){
            if(tt_found && td.flag == Egtb){
                Trans::refresh(tr,key,td.age);
                return td.eval;
            }
            else if(thread.pos.eg_eval(eval)){
                Trans::write(tr,key,depth,eval,eval,0,Exact,NullMove,ply);
                return eval;
            }
        }
    }

    // Transposition table
    if(tt_found){
        if(node == NodeMWS && (depth < 0 || (depth == 0 && td.depth == 0)
            || depth > 0)){
            if(td.flag == Exact || td.flag == Egtb){
                Trans::refresh(tr,key,td.age);
                return td.eval;
            }
            if(td.flag == Alpha && td.eval >= beta){
                Trans::refresh(tr,key,td.age);
                return td.eval;
            }
            if(td.flag == Beta && td.eval <= alpha){
                Trans::refresh(tr,key,td.age);
                return td.eval;
            }
        }
        tt_move = td.move;
    }

    is_check = thread.pos.is_in_check();

    // Mate distance pruning
    if(!is_check) eval = - MATE + (ply+2);
    else eval = - MATE + ply;

    if(alpha < eval){
        alpha = eval;
        if(beta <= eval) return eval;
    }

    eval = MATE - ply;
    if(beta > eval){
        beta = eval;
        if(alpha >= eval) return eval;
    }

    // Reevaluation
    if(!is_check){
        if(tt_found){
            re_eval = td.s_eval;
            if(td.flag == Exact) re_eval = td.eval;
            else if(td.flag == Alpha){
                if(td.eval > re_eval) re_eval = td.eval;
            }
            else if(td.flag == Beta){
                if(td.eval < re_eval) re_eval = td.eval;
            }
            if(re_eval > alpha){
                if(re_eval >= beta) return re_eval;
                alpha = re_eval;
            }
            ss->static_eval = td.s_eval;
            ss->eval_margin = td.eval_margin;
        }else{
            ss->static_eval = thread.pos.eval<false>(thread,ss->eval_margin);
            if(ss->static_eval > alpha){
                if(ss->static_eval >= beta){
                    Trans::write(tr,key,depth,ss->static_eval,ss->static_eval,
                                 ss->eval_margin,Alpha,tt_move,ply);
                    return ss->static_eval;
                }
                alpha = ss->static_eval;
            }
        }
        f_eval = ss->static_eval + 50;
    }
    else{
        ss->static_eval = thread.pos.eval<false>(thread,ss->eval_margin);
        f_eval = MATE;
        depth += Ply;
    }

    update_his_diff(ss,thread);

    if(ply == MAX_SEARCH_PLY - 1) return ss->static_eval;

    MoveGenerator mg(thread,tt_move, depth);

    if(is_check && mg.size() == 0) return -MATE+ply;

    if(node == NodePVS) pv_size = MAX_SEARCH_PLY+1-ply;

    while(alpha < beta && (move = mg.next(thread,eval)) != NullMove){
        if(node == NodeMWS && !is_check && move != tt_move
           && !thread.pos.is_capture_dangerous(move)
           && !thread.pos.is_move_check(move)){
            eval = f_eval + thread.pos.move_value(move);
            if (eval <= alpha) continue;
        }
        ss->current_move = move;
        thread.pos.move_do(move);
        eval = - qs<node>(ss+1,thread, -beta, -alpha, ply+1, depth-Ply);
        thread.pos.move_undo();
        if(should_stop(thread)) return DRAW;
        if(eval > alpha){
            ss->best_move = move;
            if(node == NodePVS){
                ss->pv[0] = move;
                memcpy(ss->pv+1,(ss+1)->pv,(pv_size-1)*sizeof(Move));
            }
            alpha = eval;
        }
    }

    if(alpha >= beta)
        Trans::write(tr,key,depth,alpha,ss->static_eval,ss->eval_margin,Alpha,
                     ss->best_move, ply);
    else if(alpha <= old_alpha)
        Trans::write(tr,key,depth,alpha,ss->static_eval,ss->eval_margin,Beta,
                     tt_move,ply);
    else
        Trans::write(tr,key,depth,alpha,ss->static_eval,ss->eval_margin,Exact,
                     ss->best_move,ply);

    return alpha;
}

template <NodeType node>
Depth Search::new_depth(const Thread & thread, const Move move,
                            const bool single_replay, bool & dangerous,
                            const Depth depth){
    dangerous = false;
    Depth result = depth - Ply;

    if(single_replay){
        dangerous = true;
        return depth;
    }

    if(thread.pos.is_move_check(move)){
        dangerous = true;
        if(thread.pos.see(move) >= SSE_GOOD){
            if(node == NodePVS) return depth;
            result += Depth(1);
        }
    }

    if(thread.pos.get_piece(move_from(move)) <= BP){
        if(thread.pos.is_passed(move_to(move))){
            dangerous = true;
            if(node == NodePVS) return depth;
            result += Depth(1);
        }
        if(rank(relative_sq(thread.pos.get_stm(),move_to(move))) == RANK_7){
            dangerous = true;
            result += Depth(1);
        }
    }

    if(node == NodePVS){
        const Piece p = thread.pos.get_piece(move_to(move));
        if (p != NO_PIECE && p > BP && thread.pos.endgame()){
            dangerous = true;
            return depth;
        }
    }

    return std::min(result,depth);
}

void Search::stop(){
    MutexLock(Search::signal_mutex);
    search_stop = true;
    MutexUnlock(Search::signal_mutex);
    Thread::stop();
}

void Search::wait(){
    Thread::wait();
}

inline bool Search::should_stop(Thread & thread){
    if(thread.should_stop || search_stop)
        return true;
    if(stop_nodes > max_stop_nodes){
        int delta_time = end_time - Engine::get_system_time();
        if(end_time > start_time && delta_time <= 0 && !ponder){
            search_stop = true;
            return true;
        }
        double time_used = Engine::get_system_time() - Search::start_time;
        if(time_used < 1) time_used = 1;
        Search::nps = int(Search::nodes/(time_used/1000.0));
        max_stop_nodes = std::min(((delta_time/2000.0)*nps), nps/10.0);
        stop_nodes = 0;
    }
    return false;
}

void Search::move_good(Thread& thread, const Move b_move, const Move moves[256],
                       const int move_nb, const int ply, const Depth depth){
    Piece p;
    Square from, to;

    if(thread.ss[ply].killer[0] != b_move){
        thread.ss[ply].killer[1] = thread.ss[ply].killer[0];
        thread.ss[ply].killer[0] = b_move;
    }

    p = thread.pos.get_piece(move_from(b_move));
    to = move_to(b_move);
    from = move_from(b_move);

    history[p][from][to] += (depth/Ply)*(depth/Ply);

    if(history[p][from][to] >= HistoryMax) {
        for(Piece p = WP; p <= BK; p++) {
            for(Square sq_f=A1; sq_f<=H8; sq_f++){
                for(Square sq_t=A1; sq_t<=H8; sq_t++){
                    history[p][sq_f][sq_t] /= 2;
                }
            }
        }
    }

    his_tot[p][from][to]++;
    his_hit[p][from][to]++;

    if (his_tot[p][from][to] >= HistoryMax) {
        his_tot[p][from][to] = (his_tot[p][from][to] + 1) / 2;
        his_hit[p][from][to] = (his_hit[p][from][to] + 1) / 2;
    }

    for(int i=0; i<move_nb-1; i++){
        if(is_tactical(moves[i])) continue;
        p = thread.pos.get_piece(move_from(moves[i]));
        to = move_to(moves[i]);
        from = move_from(moves[i]);

        his_tot[p][from][to]++;

        if(his_tot[p][from][to] >= HistoryMax) {
            his_tot[p][from][to] = (his_tot[p][from][to] + 1) / 2;
            his_hit[p][from][to] = (his_hit[p][from][to] + 1) / 2;
        }
    }
}

int Search::eval_history_move(const Thread& thread, const Move move){
    Piece p = thread.pos.get_piece(move_from(move));
    Square to = move_to(move);
    Square from = move_from(move);
    return (his_hit[p][from][to] * HistoryMax) / his_tot[p][from][to];
}

template<NodeType node>
bool Search::split_point(int & alpha, const int beta, const int ply,
                         const Depth depth, const bool is_check, int & move_nb,
                         Move * history_move, MoveGenerator * mg,
                         Thread & thread){
    split_point_available = false;

    sp.node = node;
    sp.alpha = alpha;
    sp.beta = beta;
    sp.ply = ply;
    sp.depth = depth;
    sp.is_check = is_check;
    sp.history_move = history_move;
    sp.mg = mg;
    sp.move_nb = move_nb;

    for(int i=1; i<Thread::threads_num; i++){
        Thread::thread[i]->pos = thread.pos;
        Thread::thread[i]->signal = SignalSearch;
        Thread::thread[i]->should_stop = false;
        EventSignal(Thread::thread[i]->sleep);
    }

    Search::smp_search<node>(thread,sp);

    for(int i=1; i<Thread::threads_num ; i++){
        Thread::thread[i]->signal = SignalWait;
        if(sp.alpha >= sp.beta) Thread::thread[i]->should_stop = true;
        EventWait(Thread::thread[i]->slipped);
    }

    thread.should_stop = false;
    alpha = sp.alpha;
    move_nb = sp.move_nb;
    split_point_available = true;

    return true;
}

FTYPE Search::thread_run(FARG thread) {
    Thread * t = (Thread*)thread;
    for(int i = 0; i < MAX_SEARCH_PLY; i++) t->ss[i].skip_null_move = false;
    while(true){
        switch(t->signal){
            case SignalRun:{
                for(int i = 0; i < MAX_SEARCH_PLY; i++){
                    t->ss[i].killer[0] = t->ss[i].killer[1] = NullMove;
                    t->ss[i].eMove = NullMove;
                }
                Search::ids(*t);
                EventSignal(t->slipped);
                EventWait(t->sleep);
                EventReset(t->slipped);
                break;
            }
            case SignalWait:{
                EventSignal(t->slipped);
                EventWait(t->sleep);
                EventReset(t->slipped);
                break;
            }
            case SignalSearch:{
                t->ss[sp.ply] = Thread::thread[0]->ss[sp.ply];
                for(int i = sp.ply; i < MAX_SEARCH_PLY; i++){
                    t->ss[i].killer[0] = Thread::thread[0]->ss[i].killer[0];
                    t->ss[i].killer[1] = Thread::thread[0]->ss[i].killer[1];
                    t->ss[i].eMove = NullMove;
                }
                if(sp.node == NodeMWS) Search::smp_search<NodeMWS>(*t,sp);
                else Search::smp_search<NodePVS>(*t,sp);
                if(sp.alpha >= sp.beta){
                    for(int i=0; i<Thread::threads_num ; i++)
                        Thread::thread[i]->should_stop = true;
                }
                EventSignal(t->slipped);
                EventWait(t->sleep);
                EventReset(t->slipped);
                break;
            }
            case SignalTerminate:{
                return NULL;
            }
        }
    }
    return NULL;
}
