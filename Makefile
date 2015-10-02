.PHONY: clean

CXXFLAGS = -g -W -Wall -std=c++14 `sdl2-config --cflags`
LDFLAGS = -lGL -lpng `sdl2-config --libs`

SRCS = main.cpp Renderer.cpp Mesh.cpp Loader.cpp Texture.cpp
OBJS = $(addprefix build/, $(SRCS:.cpp=.o))

PROG = helium

$(PROG): $(OBJS)
	g++ -o $(PROG) $(LDFLAGS) $(OBJS)

build/%.o: src/%.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(PROG) $(OBJS)
