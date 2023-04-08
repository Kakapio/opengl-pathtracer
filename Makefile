PROGRAMNAME=Scenegraphs
OBJS=Model.o View.o Controller.o TextRendering.o Camera.o CameraBehaviors.o $(PROGRAMNAME).o

CC=g++
CXXFLAGS=-std=c++17 -O0 -fno-inline -g
LDFLAGS=-std=c++17 -g -lglad -lglfw3
INCLUDES = -I../include
LIBS = -L../lib

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	LDFLAGS += -lopengl32 -lgdi32
	PROGRAM =$(addsuffix .exe,$(PROGRAMNAME))
else ifeq ($(shell uname -s),Darwin)     # is MACOSX
	LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit
	CC := g++-12
	PROGRAM = $(PROGRAMNAME)
endif

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS) 

%.o: %.cpp
	$(CC) $(INCLUDES) $(CXXFLAGS) -c $< -o $@


RM = rm	-f
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	RM := del
endif

clean:
	$(RM) **.o **.dSYM $(PROGRAM) **.DS_Store
