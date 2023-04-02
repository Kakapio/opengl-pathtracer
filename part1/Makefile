OBJ1=building1.o
OBJ2=building2.o
INCLUDES = -I../../include
LIBS = -L../../lib
LDFLAGS = -lglad -lglfw3
CFLAGS = -g -std=c++17
PROGRAM =
COMPILER = g++

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    LDFLAGS += -lopengl32 -lgdi32
    PROGRAM :=$(addsuffix .exe,$(PROGRAM))
else ifeq ($(shell uname -s),Darwin)     # is MACOSX
    LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit
endif

all: building1 building2

building1: $(OBJ1)
	$(COMPILER) -o building1$(PROGRAM) $(OBJ1) $(LIBS) $(LDFLAGS)

building2: $(OBJ2)
	$(COMPILER) -o building2$(PROGRAM) $(OBJ2) $(LIBS) $(LDFLAGS)

building1.o: building1.cpp BoxSet.h Box.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c building1.cpp

building2.o: building2.cpp BoxSet.h Box.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c building2.cpp

	
RM = rm	-f
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    RM := del
endif

clean: 
	$(RM) $(OBJ1) $(OBJ2) building1$(PROGRAM) building2$(PROGRAM)
    