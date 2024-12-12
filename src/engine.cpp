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

#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <algorithm>

#include "engine.h"
#include "movegen.h"
#include "search.h"
#include "thread.h"
#include "book.h"
#include "gtb.h"
#include "thread.h"

const std::string Engine::name = NAME;
const std::string Engine::author = "Borko Boskovic";
const std::string Engine::year = YEAR;
const std::string Engine::about = "http://umko.sourceforge.net/";
//const srd::string Engine::start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string Engine::start_position = "6rk/2r2pp1/B1n2n1p/P3pP1P/1p4R1/1P6/R1P4K/4B3 w - - 2 61";

#if defined(__arm__)
const std::string Engine::arch = "arm";
#endif
#if defined(__x86_64__)
const std::string Engine::arch = "x64";
#endif
#if defined(__i386__)
const std::string Engine::arch = "i686";
#endif

Position Engine::pos;
std::list<Option> Engine::option;

int Engine::wtime;
int Engine::btime;
int Engine::winc;
int Engine::binc;
int Engine::movestogo;
bool Engine::ponder = true;
int Engine::depth;
int Engine::movetime;
bool Engine::infinite;
unsigned long long Engine::nodes;
int Engine::mate;

void Engine::info(){
    std::cout<<"info string "<<name<<" "<<arch;
    std::cout<<" Copyright (C) "<<year<<" "<<author<<"\n";
    std::cout<<"info string This program comes with ABSOLUTELY NO WARRANTY.\n";
    std::cout<<"info string This is free software, and you are welcome to\n";
    std::cout<<"info string redistribute it under certain conditions (GPL 3).";
    std::cout<<std::endl;
}

void Engine::init(){
    RootMoves::init();
    Thread::init();
    MoveGenerator::init();
    Trans::create();
    pos.set_fen(start_position);
    Search::init();
    add_options();
    Book::load();
    GTB::load();
    new_game();
}

void Engine::run(){
    info();
    init();

    std::string line;
    while(getline(std::cin,line)){
        std::stringstream ss(line);
        if(!command(ss)) break;
    }
    quit();
}

bool Engine::command(std::stringstream& line)
{
    std::string command;
    line >> command;

    if(command == "stop")           { stop_search(); }
    else if(command == "ponderhit") { stop_ponder_search(); }
    else if(command == "go")        { go(line);}
    else if(command == "position")  { position(line); }
    else if(command == "setoption") { set_option(line); }
    else if(command == "ucinewgame"){ new_game(); }
    else if(command == "isready")   { std::cout<<"readyok"<<std::endl; }
    else if(command == "quit")      { return false; }
    else if(command == "uci")       { id(); }
    else if(command == "eval")      {
        Thread::thread[0]->pos = pos;
        Thread::thread[0]->pos.print_eval(*Thread::thread[0]);
    }
    else if(command == "epdtest")   { epdtest(line); }
    else if(command == "evaltest")      { eval(line); }
    else std::cerr<<"Unknown uci command: "<<command<<std::endl;

    return true;
}

void Engine::id(){
    std::cout<<"id name "<<name<<" "<<arch<<"\n";
    std::cout<<"id author "<<author<<std::endl;
    option_list();
    std::cout<<"uciok"<<std::endl;
}

int Engine::get_search_time(){
    if(movetime != 0) {
        if(Search::ponder) return 2 * movetime + get_system_time();
        return movetime + get_system_time();
    }

    if((wtime + winc + btime + binc) == 0) return 0;

    if(movestogo == 0) movestogo = 40;

    #if defined(__MINGW32__)
        wtime -= 3000;
        btime -= 3000;
    #endif

    if(movestogo == 1){
        if(pos.get_stm() == White){
            wtime = wtime * 0.9;
            if(wtime <= 10) wtime = 10;
            return wtime + get_system_time();
        }
        else{
            btime = btime * 0.9;
            if(btime <= 10) btime = 10;
            return btime + get_system_time();
        }
    }

    int cwtime, cbtime;
    if(pos.get_stm() == White){
        if(movestogo > 10 && wtime > 10000){
            cwtime = ((wtime + winc * (movestogo-1)) / movestogo) * 1.3;
            cbtime = ((btime + binc * (movestogo-1)) / movestogo) * 1.3;
            cwtime = cwtime + (cwtime - cbtime) / 2;

        }
        else{
            cwtime = ((wtime + winc * (movestogo/2)) / movestogo) * 0.9;
        }
        if(Search::ponder) cwtime = cwtime * 1.3;
        if(cwtime > wtime) cwtime = wtime * 0.9;
        if(cwtime <= 10) cwtime = 10;
        return cwtime + get_system_time();
    }
    else{
        if(movestogo > 10 && btime > 10000){
            cwtime = ((wtime + winc * (movestogo-2)) / movestogo) * 1.3;
            cbtime = ((btime + binc * (movestogo-1)) / movestogo) * 1.3;
            cbtime = cbtime + (cbtime - cwtime) / 2;
        }
        else{
            cbtime = ((btime + binc * (movestogo/2)) / movestogo) * 0.9;
        }
        if(Search::ponder) cbtime = cbtime * 1.3;
        if(cbtime > btime) cbtime = btime * 0.9;
        if(cbtime <= 10) cbtime = 10;
        return cbtime + get_system_time();
    }
}

int Engine::get_system_time(){
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec*1000 + t.tv_usec/1000;
}

void Engine::go(std::stringstream& stream){
    std::string command;
    init_search();
    stream>>command;
    while(command != ""){
        if(command == "wtime") stream>>wtime;
        else if(command == "btime") stream>>btime;
        else if(command == "winc") stream>>winc;
        else if(command == "binc") stream>>binc;
        else if(command == "movestogo") stream>>movestogo;
        else if(command == "ponder") { Search::ponder = true; }
        else if(command == "depth"){ stream>>depth;}
        else if(command == "movetime") stream>>movetime;
        else if(command == "infinite") infinite = true;
        else if(command == "nodes") stream>>nodes;
        else if(command == "mate") stream>>mate;
        else { std::cerr<<"Wrong go command: "<<command<<std::endl; return; }
        command = "";
        stream>>command;
    }
    start_search();
}

void Engine::position(std::stringstream& stream){
    std::string command = "";
    stream>>command;
    if(command == "startpos"){
        pos.start();
        command = "";
        stream>>command;
        if(command == "moves"){
            command = "";
            stream>>command;
            while(command != ""){
                pos.move_do(command);
                command = "";
                stream>>command;
            }
        }
    }
    else if(command == "fen"){
        std::string fen;
        for(int i=0; i<6; i++){
            stream>>command;
            if(i<5) fen += command + " ";
            else fen += command;
        }
        pos.set_fen(fen);
        command = "";
        stream>>command;
        if(command == "moves"){
            command = "";
            stream>>command;
            while(command != ""){
                pos.move_do(command);
                command = "";
                stream>>command;
            }
        }
    }
}

void Engine::init_search(){
    wtime = btime = 0;
    winc = binc = 0;
    movestogo = 0;
    infinite = Search::ponder = false;
    mate = depth = movetime = 0;
    nodes = 0;
}

void Engine::start_search(){
    Search::start(pos,infinite,Depth(depth*Ply),nodes);
}

void Engine::stop_search(){
    Search::stop();
    RootMoves::print_best_move();
}

void Engine::stop_ponder_search(){
    MutexLock(Search::signal_mutex);
    Search::ponder = false;
    MutexUnlock(Search::signal_mutex);
    Thread::wait();
}

void Engine::quit(){
    Search::stop();
    Trans::destroy();
    Thread::destroy();
    Book::close();
    GTB::close();
}

void Engine::add_spin_option(const char name[255], const int min, const int max,
                            int* value, void (*function)(void)){
    Option op;
    op.spin.type = SPIN;
    strcpy(op.spin.name,name);
    op.spin.function = function;
    op.spin.min = min;
    op.spin.max = max;
    op.spin.def = *value;
    op.spin.value = value;
    option.push_back(op);
}

void Engine::add_string_option(const char name[255], const char* value,
                               void (*function)(void)){
    Option op;
    op.string.type = STRING;
    strcpy(op.string.name,name);
    op.string.function = function;
    strcpy(op.string.def,value);
    op.string.value = (char *)value;
    option.push_back(op);
}

void Engine::add_button_option(const char name[255],void (*function)(void)){
    Option op;
    op.button.type = BUTTON;
    op.button.function = function;
    strcpy(op.button.name,name);
    option.push_back(op);
}

void Engine::add_check_option(const char name[255], const bool def, bool* value,
                                void (*function)(void)){
    Option op;
    op.check.type = CHECK;
    op.check.function = function;
    strcpy(op.check.name,name);
    op.check.def = def;
    op.check.value = value;
    option.push_back(op);
}

void Engine::option_list(){
    std::list<Option>::iterator it;
    Option* op;
    for(it = option.begin(); it != option.end(); it++ ){
        op = &*it;
        switch(op->spin.type){
            case SPIN:{
                std::cout<<"option name "<<op->spin.name;
                std::cout<<" type spin default "<<op->spin.def;
                std::cout<<" min "<<op->spin.min;
                std::cout<<" max "<<op->spin.max<<std::endl;
                break;
            }
            case STRING:{
                std::cout<<"option name "<<op->string.name;
                std::cout<<" type string default "<<op->string.def;
                std::cout<<std::endl;
                break;
            }
            case BUTTON:{
                std::cout<<"option name "<<op->button.name;
                std::cout<<" type button "<<std::endl;
                break;
            }
            case CHECK:{
                std::cout<<"option name "<<op->check.name;
                if(op->check.def)
                    std::cout<<" type check default true"<<std::endl;
                else
                    std::cout<<" type check default false"<<std::endl;
                break;
            }
        }
    }
}

void Engine::set_option(std::stringstream& stream){
    std::string command, name = "";
    stream >> command;
    Option *op;
    if(command == "name"){
        command = "";
        stream>>command;
        name = command;
        while(true){
            command = "";
            stream>>command;
            if(command == "value" || command == "") break;
            name += " " + command;
        }
        std::list<Option>::iterator it;
        for(it = option.begin(); it != option.end(); it++ ) {
            op = &*it;
            if(name == op->spin.name){
                switch(op->spin.type){
                    case SPIN:{
                        stream>>*op->spin.value;
                        if(*op->spin.value > op->spin.max
                           || *op->spin.value < op->spin.min){
                            std::cerr<<"Wrong uci option value(";
                            std::cerr<<name<<":"<<op->spin.value<<")";
                            std::cerr<<std::endl;
                            exit(1);
                        }
                        std::cout<<"info string "<<name;
                        std::cout<<" set to "<<*op->spin.value<<std::endl;
                        if(op->spin.function != NULL)
                            op->spin.function();
                        break;
                    }
                    case STRING:{
                        stream>>op->string.value;
                        stream.getline(op->string.value+strlen(op->string.value),200);
                        std::cout<<"info string "<<name;
                        std::cout<<" set to "<<op->string.value<<std::endl;
                        if(op->string.function != NULL)
                            op->string.function();
                        break;
                    }
                    case BUTTON:{
                        std::cout<<"info string "<<name;
                        std::cout<<" selected"<<std::endl;
                        if(op->button.function != NULL)
                            op->button.function();
                        break;
                    }
                    case CHECK:{
                        stream>>command;
                        if(command == "true")
                            *op->check.value = true;
                        else if(command == "false")
                            *op->check.value = false;
                        else{
                            std::cerr<<"Wron uci option value (";
                            std::cerr<<name<<":"<<command<<")";
                            std::cerr<<std::endl;
                            exit(1);
                        }
                        std::cout<<"info string "<<name;
                        std::cout<<" set to "<<command<<std::endl;
                        if(op->check.function != NULL)
                            op->check.function();
                        break;
                    }
                }
            }
        }
    }
}

void Engine::add_options(){
    add_spin_option("Hash",1,10240,&Trans::m_size,Trans::create);
    add_check_option("OwnBook",Book::use,&Book::use,NULL);
    add_check_option("Ponder",Engine::ponder,&Engine::ponder,NULL);
    add_string_option("Book File",Book::file_name,Book::load);
    add_string_option("GaviotaTbPath",GTB::path,GTB::load);
    add_spin_option("GaviotaTbScheme",0,4,&GTB::scheme,GTB::load);
    add_spin_option("GaviotaTbCache",1,256,&GTB::cache,GTB::load);
    add_string_option("UCI_EngineAbout", about.c_str(), NULL);
    add_spin_option("MultiPV", 1, 40, &RootMoves::multi_pv,NULL);
    add_spin_option("MultiPV delta",1,30000,&Search::delta,NULL);
    add_spin_option("SMP", 1, MAX_THREADS, &Thread::threads_num,Thread::init);
    add_spin_option("Pawn hash", 1, 4, &Thread::ph_msize,Thread::init);
    add_spin_option("Material hash", 1, 4, &Thread::mh_msize,Thread::init);
}

void Engine::new_game(){
    Trans::reset();
}

void Engine::epdtest(std::stringstream & stream){
    int solved = 0, num = 0;
    std::string filename;
    char line[1000];
    bool sl;
    stream>>filename;
    std::cout<<std::endl<<filename<<std::endl;
    std::ifstream filestream(filename.c_str());
    if(!filestream.good()){
        std::cerr<<"Error: can not open file "<<filename<<"!"<<std::endl;
        return;
    }
    std::ofstream log((filename+"_s.log").c_str());
    if(!log.good()){
        std::cerr<<"Error: can not open file "<<filename+".log"<<"!"<<std::endl;
        return;
    }
    std::ofstream epdu(filename.replace(filename.find('.'),7,"_un.epd").c_str());
    if(!epdu.good()){
        std::cerr<<"Error: can not open file ";
        std::cerr<<filename.replace(filename.find('.'),7,"_un.epd");
        std::cerr<<"!"<<std::endl;
        return;
    }
    init_search();
    stream>>movetime;
    std::streambuf* cout_sbuf = std::cout.rdbuf();
    unsigned long long sum_time = 1;

    unsigned long long nps = 0, nodes = 0, depth = 0, sdepth = 0, hash = 0;
    while(true){
        filestream.getline(line,1000);
        if(filestream.eof()) break;
        if(strlen(line) < 10) continue;
        num++;
        std::cout.rdbuf(log.rdbuf());
        sl = epdline(line);
        if(sl){
            sum_time += RootMoves::sel_time;
            std::cout<<"result solved time "<<RootMoves::sel_time<<std::endl;
        }
        else{
            sum_time += movetime;
            std::cout<<"result unsolved time "<<movetime<<std::endl;
        }
        std::cout.rdbuf(cout_sbuf);
        if(!sl) epdu<<line<<std::endl;
        else solved ++;
        nps += Search::nps;
        nodes += Search::nodes;
        depth += (Search::ids_depth/Ply)-Ply;
        sdepth += Search::max_ply;
        hash += Trans::usage();
        std::cout<<"\rnps:"<<nps/(float)num;
        std::cout<<" n:"<<nodes/(float)num;
        std::cout<<" d:"<<depth/(float)num;
        std::cout<<" sd:"<<sdepth/(float)num;
        std::cout<<" h:"<<hash/(float)num;
        std::cout<<" s:"<<solved<<"/"<<num;
        std::cout<<" rt:"<<sum_time/1000.0;
        std::cout<<"                        "<<std::flush;
    }
    std::cout<<std::endl;
    log<<"fullresult "<<"\rnps:"<<nps/(float)num;
    log<<" n:"<<nodes/(float)num<<" d:"<<depth/(float)num;
    log<<" sd:"<<sdepth/(float)num<<" h:"<<hash/(float)num;
    log<<" s:"<<solved<<"/"<<num<<" rt:"<<sum_time/1000.0<<std::endl;
    filestream.close();
    log.close();
    epdu.close();
}

bool Engine::epdline(const char line[]){
    std::string tmp, fen, move;
    std::istringstream stream(line);
    size_t index = std::string::npos;
    bool result = false;

    for(int i=0; i<4; i++){
        stream>>tmp;
        if(i==0) fen = tmp;
        else fen += " " + tmp;
    }
    do{ stream>>tmp; }while(tmp.find("bm") == std::string::npos);
    set_fen(fen.c_str());
    std::cout<<line<<std::endl;

    new_game();
    start_search();
    Search::wait();
    move = pos.move_to_san(RootMoves::get(0)->move);

    while(index == std::string::npos){
        stream>>tmp;
        index = tmp.find(';');
        if(index != std::string::npos) tmp.erase(index);
        if(move.compare(tmp) == 0){
            result = true;
            break;
        }
    }
    return result;
}

void Engine::eval(std::stringstream & stream){
    int num = 0;
    std::string filename;
    char line[1000];
    stream>>filename;
    std::cout<<std::endl<<filename<<std::endl;
    std::ifstream filestream(filename.c_str());
    if(!filestream.good()){
        std::cerr<<"Error: can not open file "<<filename<<"!"<<std::endl;
        return;
    }
    std::ofstream log((filename+"_e.log").c_str());
    if(!log.good()){
        std::cerr<<"Error: can not open file "<<filename+".log"<<"!"<<std::endl;
        return;
    }
    std::ofstream epdu(filename.replace(filename.find('.'),7,"_un.epd").c_str());
    if(!epdu.good()){
        std::cerr<<"Error: can not open file ";
        std::cerr<<filename.replace(filename.find('.'),7,"_un.epd");
        std::cerr<<"!"<<std::endl;
        return;
    }
    std::streambuf* cout_sbuf = std::cout.rdbuf();
    unsigned long long time = get_system_time();
    const int n = 1;
    while(true){
        filestream.getline(line,1000);
        if(filestream.eof()) break;
        if(strlen(line) < 10) continue;
        num++;
        std::cout.rdbuf(log.rdbuf());
        evalline(line,n);
        std::cout.rdbuf(cout_sbuf);
        //std::cout<<"\rposition: "<<num<<std::flush;
    }
    time = (get_system_time() - time)+1;
    std::cout<<std::endl<<"time: "<<time<<" nps "<<int((num*n)/(time/1000.0))<<std::endl;
    log<<std::endl<<"time: "<<time<<" nps "<<int((num*n)/(time/1000.0))<<std::endl;
    filestream.close();
    log.close();
    epdu.close();
}

void Engine::evalline(const char line[], const int n){
    std::string tmp, fen;
    std::istringstream stream(line);
    int eval = 0, margin = 0;

    for(int i=0; i<4; i++){
        stream>>tmp;
        if(i==0) fen = tmp;
        else fen += " " + tmp;
    }

    set_fen(fen.c_str());

    for(int i=0; i<n; i++)
        eval = pos.eval<false>(*Thread::thread[0],margin);

    std::cout<<line<<" e: "<<eval<<" m:"<<margin<<std::endl;
}

