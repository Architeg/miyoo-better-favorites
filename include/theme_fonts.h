#ifndef BETTER_FAVORITES_THEME_FONTS_H
#define BETTER_FAVORITES_THEME_FONTS_H
#include "theme.h"
// Call after TTF_Init. Validates actual font loading, preserving requested sizes.
void resolveThemeFonts(Theme& theme);
#endif
