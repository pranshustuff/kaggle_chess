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

#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <sstream>
#include <list>

#include "thread.h"
#include "position.h"
#include "trans.h"

#define STRING 1
#define SPIN 2
#define BUTTON 3
#define CHECK 4

class SpinOption{
public:
    int type;
    char name[255];
    void (*function)(void);
    int def;
    int min;
    int max;
    int* value;
};

class StringOption{
public:
    int type;
    char name[255];
    void (*function)(void);
    char def[255];
    char* value;
};

class ButtonOption{
public:
    int type;
    char name[255];
    void (*function)(void);
};

class CheckOption{
public:
    int type;
    char name[255];
    void (*function)(void);
    bool def;
    bool* value;
};

union Option{
    SpinOption spin;
    StringOption string;
    ButtonOption button;
    CheckOption check;
};

class Engine{
public:
    static void init();
    static void info();
    static void run();
    static void new_game();
    static void go(std::stringstream& stream);
    static void stop_search();
    static void stop_ponder_search();
    static void quit();
    static void add_spin_option(const char name[255], const int min,
                                const int max, int* value,
                                void (*function)(void));
    static void add_string_option(const char name[255], const char* value,
                                    void (*function)(void));
    static void add_button_option(const char name[255], void (*function)(void));
    static void add_check_option(const char name[255], const bool def,
                                 bool* value, void (*function)(void));
    static void option_list();
    static void set_option(std::stringstream& stream);
    static int get_system_time();
    static int get_search_time();
    inline static void set_fen(const char fen[]);
    static Position pos;
    static bool ponder;
private:
    static void init_search();
    static void start_search();
    static bool command(std::stringstream& line);
    static void id();
    static void position(std::stringstream& stream);
    static void epdtest(std::stringstream& stream);
    static void eval(std::stringstream& stream);
    static bool epdline(const char line[]);
    static void evalline(const char line[], const int n);

    static const std::string name;
    static const std::string author;
    static const std::string year;
    static const std::string about;
    static const std::string arch;
    static const std::string start_position;

    static std::list<Option> option;
    static void add_options();

    static int wtime;
    static int btime;
    static int winc;
    static int binc;
    static int movestogo;
    static int depth;
    static int movetime;
    static bool infinite;
    static unsigned long long nodes;
    static int mate;
};

inline void Engine::set_fen(const char fen[]){ pos.set_fen(fen); }

#endif // ENGINE_H
