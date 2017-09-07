
FLAGS = -O3 -std=c++11 -I../deps/glm/include/ -o grapher
FILES = evts.cpp exp.cpp gl.cpp graph.cpp main.cpp state.cpp ui.cpp
LINK = -lSDL2 -lSDL2main -lSDL2_ttf -lGL -pthread

all:
	g++ $(FLAGS) $(FILES) $(LINK)

clean:
	rm -rf *.o