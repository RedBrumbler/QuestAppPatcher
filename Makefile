SRC_PATH=src/
CC=g++ -MD
CFLAGS=-c -Wall -std=c++17
LDFLAGS=-lSDL -lSDL_gfx
INCL=-I include/
EXE=QuestAppPatcher

SRC=$(wildcard $(SRC_PATH)*.cpp)
OBJ=$(subst src, obj, $(SRC:.cpp=.o))

all: $(SRC) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) -o $@

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCL) -o $@ $<

clean:
	rm -rf $(EXE) $(OBJ)

-include $(OBJ:.o=.d)