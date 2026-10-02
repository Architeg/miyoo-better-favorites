#ifndef BETTER_FAVORITES_MENU_TEXT_H
#define BETTER_FAVORITES_MENU_TEXT_H
#include <functional>
#include <string>
#include <vector>
// Width is measured by the actual loaded font. Long words split only at UTF-8 boundaries.
std::vector<std::string> wrapMenuText(const std::string& text, int width,
                                    const std::function<int(const std::string&)>& measure);
// Complete user-facing summaries; detailed original errors remain in logs.
std::string conciseMenuError(const std::string& detail);
#endif
