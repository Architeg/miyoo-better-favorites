#ifndef BETTER_FAVORITES_MENU_STATE_H
#define BETTER_FAVORITES_MENU_STATE_H
#include <vector>
#include <cstddef>
enum class MenuPage { Browser, Welcome, Actions, Settings, ReturnInfo, HomeInfo, Help, RemoveConfirm };
enum class MenuKey { Up, Down, Left, Right, A, B, Menu, Select, Y };
enum class MenuAction { None, Launch, Remove, ToggleReturn, ToggleHome, ToggleGrouping, TogglePrefixes, CycleSorting, PageUp, PageDown };
struct MenuFrame { MenuPage page; std::size_t selected = 0; };
class MenuState {
public:
    MenuPage page() const;
    std::size_t selected() const;
    bool open() const { return !stack_.empty(); }
    void showWelcome() { push(MenuPage::Welcome); }
    void close() { stack_.clear(); }
    MenuAction handle(MenuKey key, bool repeat, bool hasFavorite);
private:
    std::vector<MenuFrame> stack_;
    void push(MenuPage page, std::size_t selected = 0);
};
#endif
