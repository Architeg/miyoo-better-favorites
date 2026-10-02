#ifndef BETTER_FAVORITES_BROWSER_MODEL_H
#define BETTER_FAVORITES_BROWSER_MODEL_H
#include "settings.h"
#include "favorites.h"
#include "ui_row.h"
#include <vector>
std::string browserDisplayLabel(const Favorite& favorite,const AppSettings& settings);
std::string browserSortKey(const std::string& label,const AppSettings& settings);
std::vector<SystemGroup> groupBrowserFavorites(const std::vector<Favorite>& favorites,const AppSettings& settings);
struct BrowserAnchor {Favorite selected,top;std::size_t ordinal=0;long first=0;bool valid=false,hasTop=false;};
BrowserAnchor captureBrowserAnchor(const std::vector<UiRow>& rows,std::size_t selected,long first);
void restoreBrowserAnchor(const BrowserAnchor& anchor,const std::vector<UiRow>& rows,std::size_t& selected,long& first);
#endif
