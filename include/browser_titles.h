#ifndef BETTER_FAVORITES_BROWSER_TITLES_H
#define BETTER_FAVORITES_BROWSER_TITLES_H
#include "title_scroll.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <map>
#include <string>
#include <utility>

// Existing row/content geometry, with the preview column reserved when present.
SDL_Rect browserTitleRegion(int rowY, int rowHeight, int previewWidth);

class BrowserTitles {
public:
    explicit BrowserTitles(TTF_Font* font) : font_(font) {}
    ~BrowserTitles() { clear(); }
    BrowserTitles(const BrowserTitles&) = delete;
    BrowserTitles& operator=(const BrowserTitles&) = delete;
    void beginFrame();
    void endFrame();
    void pause(Uint32 now) { scroll_.pause(now); }
    void restartDelay(Uint32 now) { scroll_.restart(now); }
    void draw(SDL_Surface* screen, const std::string& text, SDL_Color color,
              SDL_Rect region, int rowY, int rowHeight, bool selected,
              const std::string& identity, Uint32 now, bool menuOpen);
    void clear(); // Must run before TTF_Quit/SDL_Quit.
#ifdef BETTER_FAVORITES_TITLE_TESTING
    std::size_t rasterizations() const { return rasterizations_; }
    std::size_t cachedCount() const { return cache_.size(); }
#endif
private:
    struct Entry { SDL_Surface* surface = nullptr; bool used = false; };
    TTF_Font* font_; // Borrowed, never restyled.
    std::map<std::pair<std::string,Uint32>,Entry> cache_;
    TitleScroll scroll_;
#ifdef BETTER_FAVORITES_TITLE_TESTING
    std::size_t rasterizations_ = 0;
#endif
};
#endif
