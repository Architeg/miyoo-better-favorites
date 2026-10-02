#ifndef BETTER_FAVORITES_BROWSER_STATE_H
#define BETTER_FAVORITES_BROWSER_STATE_H
#include "ui_row.h"
#include <string>
#include <vector>

bool saveBrowserState(const std::string& path, const std::vector<UiRow>& rows,
                      std::size_t selected, long first, std::string& error);
// Missing state is an ordinary fresh start; malformed state leaves defaults intact.
bool restoreBrowserState(const std::string& path, const std::vector<UiRow>& rows,
                         std::size_t& selected, long& first, std::string& error);
#endif
