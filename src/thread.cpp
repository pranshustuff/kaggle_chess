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

#include "thread.h"
#include "search.h"

int Thread::threads_num = -1;
int Thread::old_threads_num = -1;
Thread * Thread::thread[MAX_THREADS];
int Thread::ph_msize = 1;
int Thread::mh_msize = 1;
int Thread::ph_size;
int Thread::mh_size;
#if !defined(__MINGW32__)
pthread_attr_t Thread::attr;
#endif

#define STACK_SIZE 2097152

void Thread::init(){
    if(threads_num == -1){
        #if defined(__MINGW32__)
        SYSTEM_INFO s;
        GetSystemInfo(&s);
        Thread::threads_num = std::min(int(s.dwNumberOfProcessors), MAX_THREADS);
        #else
        #if defined(_SC_NPROCESSORS_ONLN)
        Thread::threads_num = std::min(int(sysconf(_SC_NPROCESSORS_ONLN)), MAX_THREADS);
        #else
        Thread::threads_num = 1;
        #endif
        #endif
    }
    else destroy();

    old_threads_num = threads_num;

    #if !defined(__MINGW32__)
    pthread_attr_init(&attr);
    if(pthread_attr_setstacksize (&attr, STACK_SIZE)){
        std::cout<<"Can not set new stack size!"<<std::endl;
        exit(1);
    }
    #endif

    for(int i=0; i<threads_num; i++){
        thread[i] = new Thread();
        thread[i]->id = i;
        thread[i]->signal = SignalWait;
        if(!ThreadCreate(thread[i]->threadp,Search::thread_run,thread[i],attr)){
            std::cout<<"Can not create thread["<<i<<"]!"<<std::endl;
            exit(1);
        }
        EventWait(thread[i]->slipped);
    }
    MutexInitialize(Search::sp.mutex);
    Search::split_point_available = true;
}

void Thread::destroy(){
    for(int i=old_threads_num-1; i>=0; i--){
        thread[i]->signal = SignalTerminate;
        thread[i]->should_stop = true;
        EventSignal(thread[i]->sleep);
        ThreadJoin(thread[i]->threadp);
        delete thread[i];
    }
    MutexDestroy(Search::sp.mutex);
}

Thread::Thread(){
    mh_size = (mh_msize * 1048576) / sizeof(MaterialHash);
    ph_size = (ph_msize * 1048576) / sizeof(PawnHash);
    material_hash = new MaterialHash[mh_size];
    pawn_hash = new PawnHash[ph_size];
    EventInitialize(sleep);
    EventInitialize(slipped);
}

Thread::~Thread(){
    delete[] material_hash;
    delete[] pawn_hash;
    EventDestroy(sleep);
    EventDestroy(slipped);
}

void Thread::search_start(const Position & pos){
    thread[0]->pos = pos;
    thread[0]->signal = SignalRun;
    thread[0]->should_stop = false;
    EventSignal(thread[0]->sleep);
}

void Thread::stop(){
    thread[0]->signal = SignalWait;
    EventSignal(thread[0]->sleep);
    EventWait(thread[0]->slipped);
    EventReset(thread[0]->sleep);
}

void Thread::wait(){
    EventWait(thread[0]->slipped);
    EventReset(thread[0]->sleep);
}

