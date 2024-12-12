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

#ifndef TRANS_H
#define TRANS_H

#include "position.h"
#include "search.h"

class TransData{
public:
    Move move;
    int8_t depth;
    int8_t flag;
    int16_t age;
    int16_t eval;
    int16_t s_eval;
    int16_t eval_margin;
};

class TransKeys{
public:
    Key key1;
    int32_t key2;
};

class TransRecord{
public:
    Key key;
    union{
        TransData data;
        TransKeys keys;
    };
};

class Trans{
public:
    static void create();
    static void destroy();
    static void reset();
    static int usage();
    static void write(TransRecord tr[], const Key key, const Depth depth,
                      const int eval, const int seval, const int eval_margin,
                      const int flag, const Move move, const int ply);
    static void refresh(TransRecord tr[], const Key key, const int age);
    static bool find(TransRecord ** tr, const Key key, TransData& data,
                     const int ply);
    static void new_age();
    static int m_size;
    static Depth update_pv(Thread & thread, const Move move, const int eval,
                          const int flag, const int ply, Depth depth);
private:
    static TransRecord* record;
    static unsigned long long free;
    static unsigned long long size;
    static const int cluster;
    static uint16_t age;
};

#endif // TRANS_H
