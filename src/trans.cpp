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

#include "trans.h"
#include "thread.h"

#include <climits>
#include <iostream>
#include <cstring>

#define InitDepth -128
TransRecord* Trans::record = NULL;
uint16_t Trans::age;

#if __arm__
	int Trans::m_size = 16;
#elif __i386__
	int Trans::m_size = 128;
#elif __x86_64__
	int Trans::m_size = 512;
#endif

unsigned long long Trans::free;
unsigned long long Trans::size;
const int Trans::cluster = 4;

void Trans::create(){
    if(record != NULL) delete record;
    size = ((m_size * 1048576ull) / sizeof(TransRecord))-cluster;
    record = new TransRecord[size+cluster];
    free = size + cluster;
    reset();
}

void Trans::destroy(){
    delete [] record;
    size = 0;
}

int Trans::usage(){
    int r = (int)(((size+cluster-free)/(float)(size+cluster))*1000);
    if(r > 1000) r = 1000;
    return r;
}

void Trans::write(TransRecord * tr, const Key key, const Depth depth,
                  const int eval, const int seval, const int eval_margin,
                  const int flag, const Move move, const int ply){
    TransRecord * sel = NULL;
    for(int i=0; i<cluster; i++, tr++){
        if((tr->key ^ tr->keys.key1 ^ tr->keys.key2) == key){
            if(tr->data.depth > depth) return;
            sel = tr;
            break;
        }
        if(tr->data.depth == InitDepth){
            sel = tr;
            break;
        }
        if(i == 0){
            sel = tr;
            continue;
        }
        if(tr->data.age < sel->data.age || tr->data.depth < sel->data.depth){
            sel = tr;
            continue;
        }
    }
    if(tr->data.depth == InitDepth || sel->data.age < age) free --;
    TransRecord rec;
    rec.data.depth = depth;
    rec.data.flag = flag;
    rec.data.move = move;
    rec.data.age = age;
    if(eval >= MATE_EVAL) rec.data.eval = eval + ply;
    else if(eval <= - MATE_EVAL) rec.data.eval = eval - ply;
    else rec.data.eval = eval;
    rec.data.s_eval = seval;
    rec.data.eval_margin = eval_margin;
    rec.key = key ^ rec.keys.key1 ^ rec.keys.key2;
    *sel = rec;
}

void Trans::refresh(TransRecord tr[], const Key key, int age){
    TransRecord rec;
    if(Trans::age == age) return;
    for(int i=0; i<cluster; i++){
        rec = tr[i];
        if((rec.key ^ rec.keys.key1 ^ rec.keys.key2) == key){
            rec.data.age = age;
            rec.key = key ^ rec.keys.key1 ^ rec.keys.key2;
            tr[i] = rec;
            free --;
        }
    }
}

bool Trans::find(TransRecord ** tr, const Key key, TransData& data,
                 const int ply){
    *tr = &record[key % size];
    TransRecord * ttr = *tr;
    TransRecord rec;
    for(int i=0; i<cluster; i++){
        rec = *ttr;
        if((rec.key ^ rec.keys.key1 ^ rec.keys.key2) == key){
            data = rec.data;
            if(data.eval > MATE_EVAL) data.eval = data.eval - ply;
            else if(data.eval < -MATE_EVAL) data.eval = data.eval + ply;
            return true;
        }
        ttr++;
    }
    return false;
}

void Trans::new_age(){
    if(age == 32767){
        Key key;
        for(unsigned long long i=0; i<size+cluster; i++){
            key = record[i].key ^ record[i].keys.key1 ^ record[i].keys.key2;
            record[i].data.age = 0;
            record[i].key = key ^ record[i].keys.key1 ^ record[i].keys.key2;
        }
        age = 1;
    }
    else age ++;
    free = size;
}

void Trans::reset(){
    record[(size+cluster)-1].key = record[0].key = 1;
    memset(record,InitDepth,sizeof(TransRecord)*(size+cluster));
    free = size+cluster;
    age = 0;
}

Depth Trans::update_pv(Thread & thread, const Move move, const int eval,
                     const int flag, const int ply, Depth depth){
    bool found = false;
    Key key = thread.pos.get_key();
    TransRecord *tr = &record[key % size];
    for(int i=0; i<cluster; i++){
        if((tr->key ^tr->keys.key1 ^ tr->keys.key2) == key){
            found = true;
            break;
        }
        tr++;
    }
    if(depth < Depth(0)) depth  = Depth(0);
    if(found) depth = Depth(tr->data.depth);
    else {
        tr -= cluster;
        TransRecord * sel = NULL;
        for(int i=0; i<cluster; i++, tr++){
            if((tr->key ^ tr->keys.key1 ^ tr->keys.key2) == key){
                sel = tr;
                break;
            }
            if(tr->data.depth == InitDepth){
                sel = tr;
                break;
            }
            if(i == 0){
                sel = tr;
                continue;
            }
            if(tr->data.age < sel->data.age || tr->data.depth < sel->data.depth){
                sel = tr;
                continue;
            }
        }
        tr = sel;
    }
    if(!found){
        int margin;
        tr->data.depth = depth;
        tr->data.flag = flag;
        tr->data.move = move;
        if(eval >= MATE_EVAL) tr->data.eval = eval + ply;
        else if(eval <= - MATE_EVAL) tr->data.eval = eval - ply;
        else tr->data.eval = eval;
        tr->data.s_eval = thread.pos.eval<false>(thread,margin);
        tr->data.s_eval = margin;
    }
    tr->data.age = age + 1;
    tr->key = key ^ tr->keys.key1 ^ tr->keys.key2;
    return depth;
}
