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

#ifndef GTB_H
#define GTB_H

#include "position.h"
#include "gtbprobe/gtb-probe.h"

class GTB{
    public:
        static void load();
        static bool isLoaded();
        static void close();
        static bool gtb_probe_hard(const Position & pos, int &res,
                                   const int ply);
        static bool gtb_probe_soft(const Position & pos, int &res,
                                   const int ply);
        static bool gtb_probe_WDL_hard(const Position & pos, int &res,
                                       const int ply);
        static bool gtb_probe_WDL_soft(const Position & pos, int &res,
                                       const int ply);
        static int hits;
        static char path[255];
        static int cache;
        static int scheme;
    private:
        static const char ** paths;
        static bool loaded;
};

#endif // GTB_H
