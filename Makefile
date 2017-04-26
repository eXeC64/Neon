.PHONY: clean run

CXXFLAGS = -g -W -Wall -std=c++14 `sdl2-config --cflags`
LDFLAGS = -lGL -lpng `sdl2-config --libs` -lassimp

SRCS = main.cpp Renderer.cpp Model.cpp Mesh.cpp Loader.cpp Texture.cpp
OBJS = $(addprefix build/, $(SRCS:.cpp=.o))

TARGET = helium

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)
