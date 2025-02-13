CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I"./SDL2/include" -I"./glew/include" -I"./src/imgui-master"
LDFLAGS = -L"./SDL2/lib" -L"./glew/lib/Release/x64" -lSDL2 -lSDL2main -lopengl32 -lglew32 -lglu32

SRC = src/main.cpp \
      src/imgui-master/imgui.cpp \
      src/imgui-master/imgui_draw.cpp \
      src/imgui-master/imgui_widgets.cpp \
      src/imgui-master/imgui_tables.cpp \
      src/imgui-master/imgui_demo.cpp \
      src/imgui-master/backends/imgui_impl_sdl2.cpp \
      src/imgui-master/backends/imgui_impl_opengl2.cpp

OUT = build/game_engine

all:
	if not exist build mkdir build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

run: all
	$(OUT)

clean:
	rmdir /s /q build
