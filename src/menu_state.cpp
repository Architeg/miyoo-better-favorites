#include "menu_state.h"
MenuPage MenuState::page() const { return open() ? stack_.back().page : MenuPage::Browser; }
std::size_t MenuState::selected() const { return open() ? stack_.back().selected : 0; }
void MenuState::push(MenuPage page, std::size_t selected) { stack_.push_back({page, selected}); }
MenuAction MenuState::handle(MenuKey key, bool repeat, bool hasFavorite) {
    if (repeat && key != MenuKey::Up && key != MenuKey::Down) return MenuAction::None;
    if (!open()) {
        if (key == MenuKey::Select) push(MenuPage::Actions, hasFavorite ? 0 : 2);
        else if (key == MenuKey::Y) push(MenuPage::Settings);
        return MenuAction::None;
    }
    if (key == MenuKey::Menu) { close(); return MenuAction::None; }
    if (key == MenuKey::B || (key == MenuKey::Select && page() == MenuPage::Actions)) {
        stack_.pop_back(); return MenuAction::None;
    }
    // Reading/paging never moves a destructive action selection.
    if (page() == MenuPage::RemoveConfirm || page() == MenuPage::Help || page() == MenuPage::ReturnInfo) {
        if (key == MenuKey::Up) return MenuAction::PageUp;
        if (key == MenuKey::Down) return MenuAction::PageDown;
    }
    if (page() == MenuPage::RemoveConfirm) {
        if (key == MenuKey::Left) stack_.back().selected = 0;
        if (key == MenuKey::Right) stack_.back().selected = 1;
    }
    const std::size_t count = page() == MenuPage::Actions ? 4 :
        (page() == MenuPage::Settings ? 5 : 0);
    if (key == MenuKey::Up && count) {
        if (stack_.back().selected) --stack_.back().selected;
    } else if (key == MenuKey::Down && count) {
        if (stack_.back().selected + 1 < count) ++stack_.back().selected;
    } else if(page()==MenuPage::Settings && selected()<4 && (key==MenuKey::A||key==MenuKey::Left||key==MenuKey::Right)) {
        const MenuAction values[]={MenuAction::ToggleReturn,MenuAction::ToggleGrouping,MenuAction::TogglePrefixes,MenuAction::CycleSorting};
        return values[selected()];
    } else if (key == MenuKey::A) {
        if (page() == MenuPage::Actions) {
            switch (selected()) {
            case 0: return hasFavorite ? MenuAction::Launch : MenuAction::None;
            case 1: if (hasFavorite) push(MenuPage::RemoveConfirm); break;
            case 2: push(MenuPage::Settings); break;
            case 3: push(MenuPage::Help); break;
            }
        } else if (page() == MenuPage::Settings) {
            if(selected()==4)push(MenuPage::ReturnInfo);
        } else if (page() == MenuPage::RemoveConfirm) {
            if (selected()) return hasFavorite ? MenuAction::Remove : MenuAction::None;
            stack_.pop_back();
        }
    }
    return MenuAction::None;
}
