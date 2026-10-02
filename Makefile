CXX := $(CROSS_COMPILE)g++

TARGET := build/better-favorites

SRC := \
	src/main.cpp \
	src/menu_state.cpp \
	src/menu_renderer.cpp \
	src/menu_text.cpp \
	src/favorite_removal.cpp \
	src/launch_request.cpp \
	src/browser_state.cpp \
	src/app_settings.cpp \
	src/favorites_parser.cpp \
	src/ui_rows.cpp \
	src/navigation.cpp \
	src/theme_loader.cpp

SDL_ROOT := third_party/sdl2_miyoo
SDL_INC := $(SDL_ROOT)/sdl2/include
SDL_LIB_DIR := $(SDL_ROOT)/prebuilt/mini
SDL_LIB := $(SDL_LIB_DIR)/libSDL2-2.0.so.0
JSONC_LIB := $(SDL_ROOT)/examples/libjson-c.so.5

SDL_TTF_INC := third_party/sdl2_ttf/include
SDL_TTF_LIB := $(SDL_ROOT)/examples/libSDL2_ttf-2.0.so.0
SDL_IMAGE_INC := third_party/sdl2_image/include
SDL_IMAGE_LIB := $(SDL_ROOT)/examples/libSDL2_image-2.0.so.0

SDL_MIXER_INC := third_party/sdl2_mixer/include
SDL_MIXER_LIB := $(SDL_ROOT)/examples/libSDL2_mixer-2.0.so.0

CXXFLAGS := \
	-std=c++17 \
	-O2 \
	-Wall \
	-Wextra \
	-Iinclude \
	-I$(SDL_INC) \
	-Ithird_party/json-c/include \
	-I$(SDL_TTF_INC) \
	-I$(SDL_IMAGE_INC) \
	-I$(SDL_MIXER_INC)

LDFLAGS := \
	$(SDL_LIB) \
	$(SDL_TTF_LIB) \
	$(SDL_IMAGE_LIB) \
	$(SDL_MIXER_LIB) \
	$(JSONC_LIB) \
	-Wl,-rpath,'$$ORIGIN'

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC) $(SDL_LIB)
	mkdir -p build
	@echo "🔨 CXX  $(TARGET)"
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
