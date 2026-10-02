#include "menu_text.h"
#include <sstream>
std::vector<std::string> wrapMenuText(const std::string& text, int width,
                                    const std::function<int(const std::string&)>& measure) {
    std::vector<std::string> lines;
    std::istringstream paragraphs(text); std::string paragraph;
    while (std::getline(paragraphs, paragraph)) {
        std::istringstream words(paragraph); std::string word, line;
        while (words >> word) {
            if (!line.empty() && measure(line + " " + word) <= width) { line += " " + word; continue; }
            if (!line.empty()) { lines.push_back(line); line.clear(); }
            while (measure(word) > width && !word.empty()) {
                std::size_t end = 0, fitting = 0;
                do {
                    ++end;
                    while (end < word.size() && (static_cast<unsigned char>(word[end]) & 0xc0) == 0x80) ++end;
                    if (measure(word.substr(0, end)) <= width || !fitting) fitting = end;
                    else break;
                } while (end < word.size());
                lines.push_back(word.substr(0, fitting)); word.erase(0, fitting);
            }
            line = word;
        }
        if (!line.empty()) lines.push_back(line);
        else if (paragraph.empty()) lines.emplace_back();
    }
    return lines;
}

std::string conciseMenuError(const std::string& detail) {
    if(detail.find("Favorites changed")!=std::string::npos || detail.find("does not match")!=std::string::npos)
        return "Favorites changed. Nothing was removed. Reopen and try again.";
    if(detail.find("Removed, but")!=std::string::npos)
        return "Favorite removed. Storage sync failed. Backup kept.";
    if(detail.find("backup")!=std::string::npos)
        return "Backup failed. Nothing was removed. Try again.";
    if(detail.find("removal")!=std::string::npos)
        return "Removal failed. Nothing was removed. Try again.";
    if(detail.find("favorites")!=std::string::npos || detail.find("Favorites")!=std::string::npos)
        return "Cannot read favorites. Reopen and try again.";
    if(detail.find("Launch request failed")!=std::string::npos)
        return "Could not launch this game. See the app log for details.";
    if(detail.find("GameSwitcher request failed")!=std::string::npos)
        return "Could not open GameSwitcher. See the app log for details.";
    if(detail=="No game selected.") return detail;
    return "Could not save the change. See the app log for details.";
}
