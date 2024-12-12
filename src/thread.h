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
#ifndef THREAD_H
#define THREAD_H

#if defined(__MINGW32__)
#include <windows.h>
typedef CRITICAL_SECTION Mutex;
#else
#include <pthread.h>
typedef pthread_mutex_t Mutex;
#endif

#include <inttypes.h>
#include <limits.h>

#include "trans.h"
#include "position.h"

#if defined(__MINGW32__)
typedef HANDLE ThreadP;
typedef HANDLE Event;
#define MutexInitialize(mutex) InitializeCriticalSection(&mutex)
#define MutexDestroy(mutex) CloseHandle(&mutex)
#define MutexLock(mutex) EnterCriticalSection(&mutex)
#define MutexUnlock(mutex) LeaveCriticalSection(&mutex)
#define ThreadCreate(threadp,function,arg,attr) \
    ((threadp = CreateThread(NULL,0,function,(LPVOID)(arg),0,NULL)) != NULL)
#define ThreadJoin(threadp) WaitForSingleObject(threadp, INFINITE)
#define EventInitialize(event) event = CreateEvent(0, false, false, 0)
#define EventDestroy(event) CloseHandle(event)
#define EventSignal(event) SetEvent(event)
#define EventWait(event) WaitForSingleObject(event, INFINITE)
#define EventReset(event) ResetEvent(event)
#else
typedef pthread_t ThreadP;
struct Event{
    public:
        void init(){
            pthread_cond_init(&cond,NULL);
            pthread_mutex_init(&mutex,NULL);
            signal_value = false;
        }
        void destroy(){
            pthread_cond_destroy(&cond);
            pthread_mutex_destroy(&mutex);
        }
        void signal(){
            pthread_mutex_lock(&mutex);
            signal_value = true;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        }
        void wait(){
            while(true){
                pthread_mutex_lock(&mutex);
                if(signal_value == true){
                    signal_value = false;
                    pthread_mutex_unlock(&mutex);
                    break;
                }
                pthread_cond_wait(&cond,&mutex);
                pthread_mutex_unlock(&mutex);
            }
        }
        void reset(){
            pthread_mutex_lock(&mutex);
            signal_value = false;
            pthread_mutex_unlock(&mutex);
        }

    private:
        bool signal_value;
        pthread_cond_t cond;
        pthread_mutex_t mutex;
};

#define MutexInitialize(mutex) pthread_mutex_init(&mutex,NULL)
#define MutexDestroy(mutex) pthread_mutex_destroy(&mutex)
#define MutexLock(mutex) pthread_mutex_lock(&mutex)
#define MutexUnlock(mutex) pthread_mutex_unlock(&mutex)
#define ThreadCreate(threadp,function,arg,attr) \
        (!pthread_create(&threadp, &attr, function, (void*)(arg)))
#define ThreadJoin(threadp) pthread_join(threadp,NULL)
#define EventInitialize(event) event.init()
#define EventDestroy(event) event.destroy()
#define EventSignal(event) event.signal()
#define EventWait(event) event.wait();
#define EventReset(event) event.reset();
#endif

#define HistoryMax 16384
#define MAX_THREADS 16

class MoveGenerator;

class MaterialHash{
    public:
        Key key;
        int8_t flags;
        int8_t cflags[2];
        int8_t mul[2];
        int16_t phase;
        int16_t eval[2];
};

class PawnHash{
    public:
        Key key;
        int16_t eval[2];
        Bitboard passed[2];
};

class Thread{
    public:
        Thread();
        ~Thread();

        Position pos;
        SearchStack ss[MAX_SEARCH_PLY];
        MaterialHash* material_hash;
        PawnHash* pawn_hash;
        bool should_stop;
        int id;
        ThreadP threadp;

        static void init();
        static void destroy();
        static void search_start(const Position & pos);
        static void stop();
        static void wait();
        static Thread * thread[MAX_THREADS];
        static int threads_num;
        static int old_threads_num;

        static int ph_msize;
        static int mh_msize;
        static int ph_size;
        static int mh_size;

        int signal;
        #if !defined(__MINGW32__)
        static pthread_attr_t attr;
        #endif
        Event sleep;
        Event slipped;

};

#endif // THREAD_H
