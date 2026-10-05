#include "menu_state.h"
#include "home_entry_settings.h"
#include "navigation.h"
#include "ui_rows.h"
#include <cassert>
#include <iostream>
int main() {
    MenuState welcome;welcome.showWelcome();assert(welcome.page()==MenuPage::Welcome);
    assert(welcome.handle(MenuKey::A,true,true)==MenuAction::None && welcome.open());
    assert(welcome.handle(MenuKey::Down,false,true)==MenuAction::PageDown);
    welcome.handle(MenuKey::A,false,true);assert(!welcome.open());
    {
        MenuState session; HomeStatusSession cache; int hashes=0;
        auto verify=[&]{++hashes;return HomeIntegrationStatus::Available;};
        auto press=[&](MenuKey k){
            auto action=session.handle(k,false,true);
            if(!session.open())cache.close();
            if(session.page()==MenuPage::Settings)cache.get(verify,action==MenuAction::ToggleHome);
        };
        press(MenuKey::Y);assert(hashes==1);
        for(int i=0;i<5;++i)press(MenuKey::Down);
        press(MenuKey::A);assert(session.page()==MenuPage::ReturnInfo);
        press(MenuKey::B);assert(hashes==1);
        press(MenuKey::Down);press(MenuKey::A);press(MenuKey::B);assert(hashes==1);
        press(MenuKey::Up);press(MenuKey::Up);press(MenuKey::A);assert(hashes==2);
        press(MenuKey::Menu);press(MenuKey::Y);assert(hashes==3);
    }
    MenuState menu;
    auto key = [&](MenuKey k, bool favorite = true, bool repeat = false) { return menu.handle(k, repeat, favorite); };
    key(MenuKey::Select, true, true); assert(!menu.open());
    key(MenuKey::Select); assert(menu.page() == MenuPage::Actions && !menu.selected());
    assert(key(MenuKey::A, true, true) == MenuAction::None);
    assert(key(MenuKey::A) == MenuAction::Launch);
    key(MenuKey::Down); key(MenuKey::A);
    assert(menu.page() == MenuPage::RemoveConfirm && menu.selected() == 0);
    assert(key(MenuKey::A) == MenuAction::None); assert(menu.page() == MenuPage::Actions);
    key(MenuKey::A);
    assert(key(MenuKey::Down)==MenuAction::PageDown && menu.selected()==0);
    assert(key(MenuKey::Up,true,true)==MenuAction::PageUp && menu.selected()==0);
    assert(key(MenuKey::Right,true,true)==MenuAction::None && menu.selected()==0);
    key(MenuKey::Right);assert(menu.selected()==1);
    assert(key(MenuKey::Down)==MenuAction::PageDown && menu.selected()==1);
    key(MenuKey::Left);assert(menu.selected()==0);key(MenuKey::Right);
    assert(key(MenuKey::Select)==MenuAction::None && menu.page()==MenuPage::RemoveConfirm);
    assert(key(MenuKey::Y)==MenuAction::None && menu.page()==MenuPage::RemoveConfirm);
    assert(key(MenuKey::A, true, true) == MenuAction::None);
    assert(key(MenuKey::A) == MenuAction::Remove);
    assert(key(MenuKey::A, false) == MenuAction::None); // selection disappeared
    key(MenuKey::B); assert(menu.page() == MenuPage::Actions && menu.selected() == 1);
    key(MenuKey::A);assert(menu.page()==MenuPage::RemoveConfirm && menu.selected()==0);
    key(MenuKey::Right);key(MenuKey::B);assert(menu.page()==MenuPage::Actions);
    key(MenuKey::A);assert(menu.selected()==0); // Reopening always starts at Cancel.
    assert(key(MenuKey::Menu)==MenuAction::None && !menu.open());
    key(MenuKey::Select);key(MenuKey::Down);key(MenuKey::Down);key(MenuKey::A);assert(menu.page()==MenuPage::Settings);
    assert(key(MenuKey::A) == MenuAction::ToggleReturn);
    for(int i=0;i<5;++i)key(MenuKey::Down); key(MenuKey::A); assert(menu.page() == MenuPage::ReturnInfo);
    key(MenuKey::B); assert(menu.page() == MenuPage::Settings && menu.selected() == 5);
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
    key(MenuKey::Y); for(int i=0;i<4;++i)key(MenuKey::Down); key(MenuKey::A); key(MenuKey::Menu); assert(!menu.open());
    key(MenuKey::Y);
    assert(key(MenuKey::Left)==MenuAction::ToggleReturn);
    assert(key(MenuKey::Right,true,true)==MenuAction::None);
    key(MenuKey::Down); assert(key(MenuKey::A)==MenuAction::ToggleGrouping);
    assert(key(MenuKey::Left)==MenuAction::ToggleGrouping);
    key(MenuKey::Down);assert(key(MenuKey::Right)==MenuAction::TogglePrefixes);
    key(MenuKey::Down);assert(key(MenuKey::A)==MenuAction::CycleSorting);
    key(MenuKey::Down);assert(key(MenuKey::Right)==MenuAction::ToggleHome);
    assert(key(MenuKey::A,true,true)==MenuAction::None);
    key(MenuKey::Down);assert(key(MenuKey::Right)==MenuAction::None);
    key(MenuKey::Down);key(MenuKey::A);assert(menu.page()==MenuPage::HomeInfo);
    key(MenuKey::Menu);
    SystemGroup group; group.label = "GB"; group.favorites.resize(3);
    auto rows = buildUiRows({}); assert(selectableRowAtOrdinal(rows, 5) == 0);
    std::vector<SystemGroup> groups{group}; rows = buildUiRows(groups);
    assert(selectableRowAtOrdinal(rows, 1) == 2);
    assert(selectableRowAtOrdinal(rows, 3) == 3); // removed final game clamps to previous
    groups[0].favorites.erase(groups[0].favorites.begin() + 1);
    rows = buildUiRows(groups); assert(selectableRowAtOrdinal(rows, 1) == 2); // successor
    std::cout << "Menu navigation, nesting, repeat guards, cancellation, empty list and nearest selection: PASS\n";
}
