/***************************************************************************
 *   Copyright (C) 2009 by Borko Bošković                                  *
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

#define MAX_SEARCH_PLY 150

#ifndef DEPTH
#define DEPTH
typedef int Depth;
#define Ply 2
#define MAX_SEARCH_DEPTH 127
#endif

#ifndef SEARCH_H
#define SEARCH_H

#include <ctime>
#include "position.h"
#include "movegen.h"

#if defined(__MINGW32__)
#include <windows.h>
typedef CRITICAL_SECTION Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t Mutex;
#endif


class Thread;
class SearchStack;

enum NodeType { NodePVS = 0, NodeMWS = 1 };

class SplitPoint{
    public:
        int alpha;
        int beta;
        int ply;
        Depth depth;
        MoveGenerator * mg;
        bool is_check;
        Move * history_move;
        Move * new_pv;
        int move_nb;
        Mutex mutex;
        NodeType node;
};

typedef int SearchFlag;
#define Alpha 0
#define Beta 1
#define Exact 2
#define Egtb 3

class SearchStack{
    public:
        Move killer[2];
        Move best_move;
        Move current_move;
        Move eMove;
        bool skip_null_move;
        int static_eval;
        int eval_margin;
        Depth reduction;
        Key key;
        Move pv[MAX_SEARCH_PLY+1];
        template <NodeType node> void init(Key key){
            reduction = Depth(0);
            current_move = best_move = NullMove;
            if(this->key != key){
                static_eval = -MATE;
                eval_margin = 0;
            }
            this->key = key;
            if(node == NodePVS) pv[0] = NullMove;
        }
};

#include "rootmoves.h"

#define SignalRun 1
#define SignalWait 2
#define SignalSearch 3
#define SignalTerminate 4

#if defined(__MINGW32__)
#define FTYPE DWORD WINAPI
#define FARG LPVOID
#else
#define FTYPE void *
#define FARG void *
#endif


class Search{
public:
    static void init();
    static void reset();
    static void start(const Position& pos, const bool infinite,
                      const Depth max_depth,
                      const unsigned long long max_nodes);
    static void ids(Thread & thread);
    static int rs(SearchStack* ss, Thread & thread, int alpha, int beta,
                  const Depth depth);
    template <NodeType node>
    static int search(SearchStack* ss, Thread & thread, int alpha, int beta,
                       const int ply, const Depth depth);
    template <NodeType node>
    static void smp_search(Thread & thread, SplitPoint & sp);
    template <NodeType node>
    static int qs(SearchStack* ss, Thread & thread, int alpha, int beta,
                  const int ply, Depth depth);
    template <NodeType>
    static Depth new_depth(const Thread & thread, const Move move,
                           const bool single_replay, bool & dangerous,
                           const Depth depth);
    template<NodeType node>
    static bool split_point(int & alpha, const int beta, const int ply,
                             const Depth depth, const bool is_check, int & move_nb,
                             Move * history_move, MoveGenerator * mg,
                             Thread & thread);
    static void stop();
    static void wait();
    static void move_good(Thread & thread, const Move b_move,
                          const Move h_move[256], const int h_size,
                          const int ply, const Depth depth);
    static int eval_history_move(const Thread& thread, const Move move);

    static int start_time;
    static int end_time;
    static int nps;
    static unsigned long long nodes;
    static int max_ply;
    static Depth ids_depth;
    static bool infinite;
    static unsigned long long max_nodes;
    static Depth max_depth;
    static bool egtb_root;
    static Depth egtb_depth;

    static bool should_stop(Thread & thread);
    static bool search_stop;
    static int stop_nodes;
    static int max_stop_nodes;

    static int delta;
    static int16_t history[14][64][64];
    static int16_t his_tot[14][64][64];
    static int16_t his_hit[14][64][64];
    static int16_t his_diff[14][64];
    static int16_t his_eval_array[16*Ply];
    static int8_t reduction_array[2][MAX_SEARCH_DEPTH][64];
    static int16_t futility_margin_array[7*Ply][64];
    static int16_t futility_move_nb_array[16*Ply];
    static int16_t razor_margin_array[4*Ply];
    static const int iid_depth[2];
    static const int singular_ext_depth[2];
    template <NodeType node>
    static Depth reduction(const Depth depth, const int mn);
    inline static int futility_margin(const Depth depth, const int mn);
    inline static void update_his_diff(const SearchStack* ss,
                                       const Thread & thread);
    inline static int get_his_diff(const Thread & thread, const Move move);
    inline static int razor_margin(const Depth depth);
    inline static int futility_move_nb(const Depth depth);
    inline static int his_eval(const Depth depth);

    static bool ponder;
    static Mutex signal_mutex;

    static Key E_Key;

    static bool split_point_available;
    static SplitPoint sp;
    static FTYPE thread_run(FARG thread);
};

#endif // SEARCH_H
