#ifndef BETTER_FAVORITES_PROFILED_SDL_H
#define BETTER_FAVORITES_PROFILED_SDL_H
#include "startup_profile.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
inline SDL_Surface* profiledImageLoad(const char* path) {
    startup_profile::Scope sample("image.decode",path);
    return IMG_Load(path);
}
inline TTF_Font* profiledFontOpen(const char* path,int size) {
    startup_profile::Scope sample("font.open",path);
    return TTF_OpenFont(path,size);
}
#endif
