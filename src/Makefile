 ###########################################################################
 #   Copyright (C) 2009-2010 by Borko Boskovic                             #
 #   borko.boskovic@gmail.com                                              #
 #                                                                         #
 #   This program is free software: you can redistribute it and/or modify  #
 #   it under the terms of the GNU General Public License as published by  #
 #   the Free Software Foundation, either version 3 of the License, or     #
 #   (at your option) any later version.                                   #
 #                                                                         #
 #   This program is distributed in the hope that it will be useful,       #
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 #   GNU General Public License for more details.                          #
 #                                                                         #
 #   You should have received a copy of the GNU General Public License     #
 #   along with this program.  If not, see <http://www.gnu.org/licenses/>. #
 ###########################################################################


XBOARD_FLAGS =  -size large -debugMode false -pgnExtendedInfo true\
                -showCoords true -highlightLastMove true \
                -hideThinkingFromHuman false -highlightSquareColor yellow \
                -darkSquareColor "\#aa5555" -lightSquareColor "\#ffcc99" \
                -whitePieceColor "\#ffffd7" -blackPieceColor "\#1d1d1d" \
                -usePolyglotBook true -polyglotBook ./performance.bin \
                -adjudicateLossThreshold -400 -defaultHashSize 512 \
		-usePolyglotBook true -polyglotBook performance.bin

SEEK =  say thanks\nseek 2 0 2000-9999\nseek 5 0 2000-9999\n\
        seek 15 0 2000-9999\n

DATE    = $(shell date '+%d_%m_%G')

NAME = MinkoChess
VERSION = 1.3
TARGET = $(NAME)_$(VERSION)
YEAR = 2009-2012
ENGINE_CXXFLAGS = -D NAME='"$(NAME) $(VERSION)"' -D YEAR='"$(YEAR)"'
CXX = g++
CXXFLAGS = -Wall -W
LIBS = ./gtbprobe/libgtb.a

ifeq ($(DEBUG),yes)
	CXXFLAGS += -O0 -g
else
	CXXFLAGS += -O3 -fno-exceptions -fno-rtti -g0 -s -DNDEBUG
	ifneq ($(OS),osx)
        	LDFLAGS += -s
	endif
endif

ifeq ($(STATIC),yes)
	CXXFLAGS += -static
	LDFLAGS += -static
endif

ifeq ($(ARCH),x64)
	CXXFLAGS += -m64
	LDFLAGS += -m64
endif

ifeq ($(ARCH),x64_SSE4)
	CXXFLAGS += -m64 -DSSE4
	LDFLAGS += -m64
endif

ifeq ($(ARCH),i686)
	CXXFLAGS += -m32 -march=i686
	LDFLAGS += -m32 -march=i686
endif

ifeq ($(OS),windows)
	ifeq ($(ARCH),x64)
		CXX = x86_64-w64-mingw32-g++
	endif
	ifeq ($(ARCH),x64_SSE4)
        	CXX = x86_64-w64-mingw32-g++
	endif
	ifeq ($(ARCH),i686)
		CXX = i586-pc-mingw32-g++
	endif
	TARGET = $(NAME)_$(VERSION)_$(ARCH).exe
endif

ifeq ($(OS),linux)
	LIBS += -lpthread
	ifeq ($(ARCH),i686)
		LDFLAGS += -L/usr/lib32/debug/
	endif
	TARGET = $(NAME)_$(VERSION)_$(ARCH)
endif

ifeq ($(OS),osx)
	LIBS += -lpthread
	TARGET = $(NAME)_$(VERSION)_$(ARCH)
endif

ifeq ($(OS),android)
	CXX = arm-mandriva-linux-gnueabi-c++
	TARGET = $(NAME)_$(VERSION)_$(ARCH)
endif

OBJECTS = main.o search.o engine.o book.o trans.o pos_move.o pos_eval.o\
          pos_gen.o pos_init.o pos_core.o movegen.o movelist.o thread.o \
          rootmoves.o pos_eg_eval.o gtb.o piece.o bitboard.o square.o move.o

OBJ = $(addprefix obj/, $(OBJECTS))

all: engine

engine: /gtbprobe/libgtb.a obj_dir $(OBJ)
	$(CXX) $(LDFLAGS) -o "$(TARGET)" $(OBJ) $(LIBS)

obj/engine.o: engine.cpp *.h
	$(CXX) $(CXXFLAGS) $(ENGINE_CXXFLAGS) -c $< -o $@

obj/%.o : %.cpp *.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj_dir:
	mkdir -p obj

/gtbprobe/libgtb.a:
	make -C gtbprobe OS=$(OS) ARCH=$(ARCH) STATIC=$(STATIC) DEBUG=$(DEBUG)

clean:
	make -C gtbprobe clean
	rm -rf obj "$(NAME)"* *~ gtb

stockfish: engine
	mkdir -p games/
	ln -fs ~/Programi/gtb
	xboard -tc 1:00 -mg 100 $(XBOARD_FLAGS) -ponderNextMove false \
		-fd . -fUCI -fcp ./$(TARGET) -sd . -sUCI -scp stockfish \
		-sgf games/stockfish_$(DATE).pgn 

umkoold: engine
	mkdir -p games/ 
	ln -fs ~/Programi/gtb 
	xboard -tc 5:00 -mg 100 $(XBOARD_FLAGS) -ponderNextMove false \
		-fd . -fUCI -fcp ./$(TARGET) -sd . -sUCI -scp OldProgram \
		-sgf games/umko_$(DATE).pgn -debug

freechess.org: engine 
	mkdir -p games/
	ln -fs ~/Programi/gtb
	xboard -size large -zippyPlay True -icslogon login -xrm '*zippyGameEnd: $(SEEK)' \
		-ics -icshost freechess.org -icshelper timeseal \
		-fd . -fUCI -fcp ./$(TARGET) $(XBOARD_FLAGS) -ponderNextMove true \
		-sgf games/freechess.org_$(DATE).pgn -autoCallFlag true \
		-autoComment true -autoKibitz -showThinking true

help:
	@echo "make OS=os ARCH=arch DEBUG=debug STATIC=static"
	@echo "os=[linux|osx|windows|android]"
	@echo "arch=[x64_SSE4|x64|i686|arm]"
	@echo "debug=[yes|no]"
	@echo "static=[yes|no]"
