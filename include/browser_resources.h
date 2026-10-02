#ifndef BETTER_FAVORITES_BROWSER_RESOURCES_H
#define BETTER_FAVORITES_BROWSER_RESOURCES_H
#include "theme.h"
#include <SDL.h>
#include <string>
// Startup-only theme decoding: configured profile -> active theme -> Miyoo fallback.
SDL_Surface* loadThemeImage(const Theme& theme, const std::string& resolvedPath);
SDL_Surface* createThemeBackground(const Theme& theme);
SDL_Surface* createThemeSelection(const Theme& theme);
// The existing selected-path cache, extracted so no-stale-art behavior is testable.
void updateFavoriteArtwork(SDL_Surface*& surface, std::string& currentPath, const std::string& newPath);
SDL_Rect fitFavoriteArtwork(int imageWidth, int imageHeight, int previewWidth);
#endif
