.PHONY: clean run

CXXFLAGS = -g -W -Wall -std=c++14 `sdl2-config --cflags` -Ithirdparty/imgui
LDFLAGS = -lGL -lpng `sdl2-config --libs` -lassimp

SRCS = $(wildcard src/*.cpp) $(wildcard thirdparty/imgui/*.cpp)
OBJS = $(addprefix build/, $(notdir $(SRCS:.cpp=.o)))

TARGET = neon

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

build/%.o: thirdparty/imgui/%.cpp
	$(CXX) -o $@ -c $<

baker: baker_src/main.cpp
	$(CXX) -o $@ $^ -lassimp

clean:
	$(RM) $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)
