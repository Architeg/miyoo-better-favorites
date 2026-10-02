#include "menu_state.h"
#include "navigation.h"
#include "ui_rows.h"
#include <cassert>
#include <iostream>
int main() {
    MenuState menu;
    auto key = [&](MenuKey k, bool favorite = true, bool repeat = false) { return menu.handle(k, repeat, favorite); };
    key(MenuKey::Select, true, true); assert(!menu.open());
    key(MenuKey::Select); assert(menu.page() == MenuPage::Actions && !menu.selected());
    assert(key(MenuKey::A, true, true) == MenuAction::None);
    assert(key(MenuKey::A) == MenuAction::Launch);
    key(MenuKey::Down); key(MenuKey::A);
    assert(menu.page() == MenuPage::RemoveConfirm && menu.selected() == 0);
    assert(key(MenuKey::A) == MenuAction::None); assert(menu.page() == MenuPage::Actions);
    key(MenuKey::A); key(MenuKey::Down);
    assert(key(MenuKey::A, true, true) == MenuAction::None);
    assert(key(MenuKey::A) == MenuAction::Remove);
    assert(key(MenuKey::A, false) == MenuAction::None); // selection disappeared
    key(MenuKey::B); assert(menu.page() == MenuPage::Actions && menu.selected() == 1);
    key(MenuKey::Down); key(MenuKey::A); assert(menu.page() == MenuPage::Settings);
    assert(key(MenuKey::A) == MenuAction::ToggleReturn);
    key(MenuKey::Down); key(MenuKey::A); assert(menu.page() == MenuPage::ReturnInfo);
    key(MenuKey::B); assert(menu.page() == MenuPage::Settings && menu.selected() == 1);
    key(MenuKey::B); assert(menu.page() == MenuPage::Actions && menu.selected() == 2);
    key(MenuKey::Down); key(MenuKey::A); assert(menu.page() == MenuPage::Help);
    key(MenuKey::Menu, true, true); assert(menu.open());
    key(MenuKey::Menu); assert(!menu.open());
    key(MenuKey::Y); assert(menu.page() == MenuPage::Settings);
    key(MenuKey::B); assert(!menu.open()); // Y opened directly: one back level
    key(MenuKey::Select, false); assert(menu.selected() == 2);
    key(MenuKey::Up, false); assert(key(MenuKey::A, false) == MenuAction::None);
    key(MenuKey::Up, false); assert(key(MenuKey::A, false) == MenuAction::None);
    assert(menu.page() == MenuPage::Actions);
    key(MenuKey::Menu, false); assert(!menu.open());
    key(MenuKey::Y); key(MenuKey::Down); key(MenuKey::A); key(MenuKey::Menu); assert(!menu.open());
    SystemGroup group; group.label = "GB"; group.favorites.resize(3);
    auto rows = buildUiRows({}); assert(selectableRowAtOrdinal(rows, 5) == 0);
    std::vector<SystemGroup> groups{group}; rows = buildUiRows(groups);
    assert(selectableRowAtOrdinal(rows, 1) == 2);
    assert(selectableRowAtOrdinal(rows, 3) == 3); // removed final game clamps to previous
    groups[0].favorites.erase(groups[0].favorites.begin() + 1);
    rows = buildUiRows(groups); assert(selectableRowAtOrdinal(rows, 1) == 2); // successor
    std::cout << "Menu navigation, nesting, repeat guards, cancellation, empty list and nearest selection: PASS\n";
}
