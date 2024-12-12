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

#ifndef ROOTMOVES_H
#define ROOTMOVES_H

#include "thread.h"
#include "move.h"
#include "thread.h"

class Thread;

class RootMove{
public:
    Move move;
    int eval;
    int depth;
    int flag;
    int time;
    Move pv[MAX_SEARCH_PLY+1];
};

class RootMoves{
public:
    static void init();
    static void new_search(const Thread& thread, const Move tt_move);
    static RootMove* get_next(const int alpha, const int beta,
                              const Depth depth);
    static bool first();
    static bool pv();
    static int pv_alpha();
    static RootMove* update(const SearchStack * ss, const int eval,
                            const int flag, const Depth depth);
    static void set_best_move(const Move move);
    static RootMove* get(const int i);
    static int get_size();
    static void print_current_move();
    static void print_best_line();
    static void print_best_move();
    static void print();
    static int multi_pv;
    static int move_number();
    static int sel_time;
    static void update_mpv_in_tt(Thread & thread, const Depth depth);
    static void update_pv_in_tt(Thread & thread, const RootMove & rm,
                                Depth depth);
private:
    static int size;
    static int current;
    static RootMove rmove[300];
    static Mutex bm_mutex;
    static bool bm_returned;
    static Move ponder_move;
};

#endif // ROOTMOVES_H
