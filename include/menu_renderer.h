#ifndef BETTER_FAVORITES_MENU_RENDERER_H
#define BETTER_FAVORITES_MENU_RENDERER_H
#include "menu_state.h"
#include "theme.h"
#include "settings.h"
#include <map>
#include <SDL.h>
#include <SDL_ttf.h>
struct MenuResources {
    SDL_Surface *background, *title, *footer, *selection;
    TTF_Font *titleFont, *listFont, *bodyFont, *hintFont;
};
class MenuRenderer {
public:
    MenuRenderer(const Theme& theme, MenuResources resources);
    ~MenuRenderer();
    MenuRenderer(const MenuRenderer&) = delete;
    MenuRenderer& operator=(const MenuRenderer&) = delete;
    void draw(SDL_Surface* screen, MenuPage page, std::size_t selected,
              bool hasFavorite, bool returnOn, bool available,
              const std::string& gameTitle, Uint32 ticks, const std::string& error = "", const AppSettings& settings = {});
    void drawError(SDL_Surface* screen, const std::string& message, Uint32 ticks);
    void movePage(int direction); // Read-only Help/explanation/title presentation pages only.
    void resetPage() { page_ = 0; previous_ = MenuPage::Browser; }
    void release();
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
    void drawControlSamples(SDL_Surface* screen);
    void verifyFonts(bool expectRegular = false) const;
    bool explanationsAreRegular() const;
#endif
    int pageCount() const { return pages_; }
private:
    TTF_Font* sectionHeadingFont_ = nullptr; // Borrowed browser heading face; never restyled.
    Theme theme_;
    MenuResources resources_;
    SDL_Surface *dialog_ = nullptr, *divider_ = nullptr, *shade_ = nullptr;
    bool usePopup_ = false;
    SDL_Color backgroundColor_ {}, panelColor_ {};
    std::map<std::string, SDL_Surface*> controls_;
    SDL_Surface* controlLabel(const std::string& key);
    SDL_Surface *popup_ = nullptr, *leftArrow_ = nullptr, *rightArrow_ = nullptr;
    TTF_Font *ownedBodyFont_ = nullptr, *ownedHintFont_ = nullptr, *descriptionFont_ = nullptr, *regularBodyFont_ = nullptr;
    MenuPage previous_ = MenuPage::Browser;
    int page_ = 0, pages_ = 1;
};
#endif
