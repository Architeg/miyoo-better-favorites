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
    const std::size_t count = page() == MenuPage::Actions ? 4 :
        (page() == MenuPage::Settings || page() == MenuPage::RemoveConfirm ? 2 : 0);
    if (key == MenuKey::Up && count) {
        if (stack_.back().selected) --stack_.back().selected;
    } else if (key == MenuKey::Down && count) {
        if (stack_.back().selected + 1 < count) ++stack_.back().selected;
    } else if (key == MenuKey::A) {
        if (page() == MenuPage::Actions) {
            switch (selected()) {
            case 0: return hasFavorite ? MenuAction::Launch : MenuAction::None;
            case 1: if (hasFavorite) push(MenuPage::RemoveConfirm); break;
            case 2: push(MenuPage::Settings); break;
            case 3: push(MenuPage::Help); break;
            }
        } else if (page() == MenuPage::Settings) {
            if (!selected()) return MenuAction::ToggleReturn;
            push(MenuPage::ReturnInfo);
        } else if (page() == MenuPage::RemoveConfirm) {
            if (selected()) return hasFavorite ? MenuAction::Remove : MenuAction::None;
            stack_.pop_back();
        }
    }
    return MenuAction::None;
}
