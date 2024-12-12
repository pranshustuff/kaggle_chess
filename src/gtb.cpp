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

#include "gtb.h"
#include <cstring>

#if defined(__MINGW32__)
char GTB::path[255] = ".\\gtb\\";
#else
char GTB::path[255] = "./gtb/";
#endif

int GTB::hits;
#if __arm__
int GTB::cache = 4;
#elif __i386__
int GTB::cache = 32;
#elif __x86_64__
int GTB::cache = 64;
#endif

const char ** GTB::paths = NULL;
bool GTB::loaded = false;
int GTB::scheme = tb_CP4;

void GTB::load(){
    char *initinfo;
    int verbosity = 1;
    int wdl_fraction = 112; // MAX 128
    size_t cache_size = cache * 1024 * 1024;

    if(paths != NULL){
        close();
        std::cout<<"info string Gaviota tablebases are closed"<<std::endl;
    }

    paths = tbpaths_init();
    paths = tbpaths_add(paths, path);

    initinfo = tb_init(verbosity, scheme, paths);
    tbcache_init(cache_size, wdl_fraction);
    tbstats_reset();
    if(initinfo != NULL) std::cout<<initinfo;
    if(strstr(initinfo,"PASSED") != NULL){
        loaded = true;
        std::cout<<"info string Gaviota tablebases are loaded"<<std::endl;
    }
    else loaded = false;
}

bool GTB::isLoaded(){
    return loaded;
}

void GTB::close(){
    tbcache_done();
    tb_done();
    paths = tbpaths_done(paths);
    loaded = false;
}

bool GTB::gtb_probe_hard(const Position & pos, int &res, const int ply){
    unsigned int stm, castles, tb_res, tb_ply, epsq, ws[5], bs[5];
    unsigned char wp[5], bp[5];

    if(pos.get_stm() == White) stm = tb_WHITE_TO_MOVE;
    else stm = tb_BLACK_TO_MOVE;

    epsq = pos.get_ep();
    castles = pos.get_castle();
    pos.gtb_info(ws,bs,wp,bp);

    bool tb_available = tb_probe_hard(stm,epsq,castles,ws,bs,wp,bp,
                                      &tb_res,&tb_ply);

    if (tb_res == tb_DRAW) res = DRAW;
    else if (tb_res == tb_WMATE && stm == tb_WHITE_TO_MOVE)
        res = MATE - (tb_ply + ply);
    else if (tb_res == tb_BMATE && stm == tb_BLACK_TO_MOVE)
        res = MATE - (tb_ply + ply);
    else if (tb_res == tb_WMATE && stm == tb_BLACK_TO_MOVE)
        res = - MATE + (tb_ply + ply);
    else if (tb_res == tb_BMATE && stm == tb_WHITE_TO_MOVE)
        res = - MATE + (tb_ply + ply);

    if(tb_available) hits ++;

    return tb_available;
}

bool GTB::gtb_probe_soft(const Position & pos, int &res, const int ply){
    unsigned int stm, castles, tb_res, epsq, ws[5], bs[5], tb_ply;
    unsigned char wp[5], bp[5];

    if(pos.get_stm() == White) stm =tb_WHITE_TO_MOVE;
    else stm = tb_BLACK_TO_MOVE;

    epsq = pos.get_ep();
    castles = pos.get_castle();
    pos.gtb_info(ws,bs,wp,bp);

    bool tb_available = tb_probe_soft(stm,epsq,castles,ws,bs,wp,bp,
                                      &tb_res,&tb_ply);

    if (tb_res == tb_DRAW) res = DRAW;
    else if (tb_res == tb_WMATE && stm == tb_WHITE_TO_MOVE)
        res = MATE - (ply + tb_ply);
    else if (tb_res == tb_BMATE && stm == tb_BLACK_TO_MOVE)
        res = MATE - (ply + tb_ply);
    else if (tb_res == tb_WMATE && stm == tb_BLACK_TO_MOVE)
        res = - MATE + (ply + tb_ply);
    else if (tb_res == tb_BMATE && stm == tb_WHITE_TO_MOVE)
        res = - MATE + (ply + tb_ply);

    if(tb_available) hits ++;

    return tb_available;
}

bool GTB::gtb_probe_WDL_hard(const Position & pos, int &res, const int ply){
    unsigned int stm, castles, tb_res, epsq, ws[5], bs[5];
    unsigned char wp[5], bp[5];

    if(pos.get_stm() == White) stm = tb_WHITE_TO_MOVE;
    else stm = tb_BLACK_TO_MOVE;

    epsq = pos.get_ep();
    castles = pos.get_castle();
    pos.gtb_info(ws,bs,wp,bp);

    bool tb_available = tb_probe_WDL_hard(stm,epsq,castles,ws,bs,wp,bp,&tb_res);

    if (tb_res == tb_DRAW) res = DRAW;
    else if (tb_res == tb_WMATE && stm == tb_WHITE_TO_MOVE)
        res = MATE_EVAL - ply;
    else if (tb_res == tb_BMATE && stm == tb_BLACK_TO_MOVE)
        res = MATE_EVAL - ply;
    else if (tb_res == tb_WMATE && stm == tb_BLACK_TO_MOVE)
        res = - MATE_EVAL + ply;
    else if (tb_res == tb_BMATE && stm == tb_WHITE_TO_MOVE)
        res = - MATE_EVAL + ply;

    if(tb_available) hits ++;

    return tb_available;
}

bool GTB::gtb_probe_WDL_soft(const Position & pos, int &res, const int ply){
    unsigned int stm, castles, tb_res, epsq, ws[5], bs[5];
    unsigned char wp[5], bp[5];

    if(pos.get_stm() == White) stm =tb_WHITE_TO_MOVE;
    else stm = tb_BLACK_TO_MOVE;

    epsq = pos.get_ep();
    castles = pos.get_castle();
    pos.gtb_info(ws,bs,wp,bp);

    bool tb_available = tb_probe_WDL_soft(stm,epsq,castles,ws,bs,wp,bp,&tb_res);

    if (tb_res == tb_DRAW) res = DRAW;
    else if (tb_res == tb_WMATE && stm == tb_WHITE_TO_MOVE)
        res = MATE_EVAL - ply;
    else if (tb_res == tb_BMATE && stm == tb_BLACK_TO_MOVE)
        res = MATE_EVAL - ply;
    else if (tb_res == tb_WMATE && stm == tb_BLACK_TO_MOVE)
        res = - MATE_EVAL + ply;
    else if (tb_res == tb_BMATE && stm == tb_WHITE_TO_MOVE)
        res = - MATE_EVAL + ply;

    if(tb_available) hits ++;

    return tb_available;
}
