.PHONY: clean

CXXFLAGS = -W -Wall -std=c++14 `sdl2-config --cflags`
LDFLAGS = -lGL `sdl2-config --libs`

SRCS = main.cpp Renderer.cpp Mesh.cpp MeshLoader.cpp
OBJS = $(SRCS:.cpp=.o)

PROG = helium

$(PROG): $(OBJS)
	g++ -o $(PROG) $(LDFLAGS) $(OBJS)

%.o: %.cpp
	g++ $(CXXFLAGS) -c $<

clean:
	rm -f $(PROG) $(OBJS)
