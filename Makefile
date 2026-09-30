CXX := $(CROSS_COMPILE)g++

TARGET := build/better-favorites

SRC := src/main.cpp

SDL_ROOT := third_party/sdl2_miyoo
SDL_INC := $(SDL_ROOT)/sdl2/include
SDL_LIB_DIR := $(SDL_ROOT)/prebuilt/mini
SDL_LIB := $(SDL_LIB_DIR)/libSDL2-2.0.so.0
JSONC_LIB := $(SDL_ROOT)/examples/libjson-c.so.5

CXXFLAGS := \
	-std=c++17 \
	-O2 \
	-Wall \
	-Wextra \
	-Iinclude \
	-I$(SDL_INC) \
	-Ithird_party/json-c/include

LDFLAGS := \
	$(SDL_LIB) \
	$(JSONC_LIB) \
	-Wl,-rpath,'$$ORIGIN'

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(SDL_LIB)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
