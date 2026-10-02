#include "browser_state.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace {
constexpr std::size_t limit = 65536;
std::string field(const std::string& value) {
    return std::to_string(value.size()) + "\n" + value + "\n";
}
bool number(std::istream& input, std::size_t& value) {
    return bool(input >> value) && value <= 1000000 && input.get() == '\n';
}
bool readField(std::istream& input, std::string& value) {
    std::size_t count = 0;
    if (!number(input, count) || count > limit) return false;
    value.resize(count);
    if (count) input.read(&value[0], count);
    return bool(input) && input.get() == '\n';
}
std::string headerSystem(const std::vector<UiRow>& rows, std::size_t index) {
    if (index + 1 < rows.size() && rows[index + 1].favorite)
        return rows[index + 1].favorite->systemId;
    return {};
}
}

bool saveBrowserState(const std::string& path, const std::vector<UiRow>& rows,
                      std::size_t selected, long first, std::string& error) {
    error.clear();
    if (selected >= rows.size() || !rows[selected].favorite || first < 0 ||
        static_cast<std::size_t>(first) >= rows.size()) {
        error = "Invalid browser position.";
        return false;
    }
    const auto& favorite = *rows[selected].favorite;
    const auto& top = rows[static_cast<std::size_t>(first)];
    if (top.type == UiRowType::Favorite && !top.favorite) {
        error = "Invalid browser viewport row."; return false;
    }
    const bool header = top.type == UiRowType::SystemDivider;
    const std::string body = "BetterFavoritesBrowserState1\n" +
        std::to_string(rows[selected].favoriteIndex) + "\n" +
        std::to_string(first) + "\n" + (header ? "1\n" : "0\n") +
        field(favorite.launchPath) + field(favorite.romPath) +
        field(header ? headerSystem(rows, first) : top.favorite->launchPath) +
        field(header ? "" : top.favorite->romPath);
    if (body.size() > limit) { error = "Browser state too large."; return false; }
    struct stat existing {};
    if (lstat(path.c_str(), &existing) == 0) {
        if (!S_ISREG(existing.st_mode)) {
            error = "Browser state is not a regular file.";
            return false;
        }
    } else if (errno != ENOENT) {
        error = "Cannot inspect browser state: " + std::string(std::strerror(errno));
        return false;
    }
    std::string temporary = path + ".XXXXXX";
    const int fd = mkstemp(&temporary[0]);
    if (fd < 0) { error = "Cannot create browser state: " + std::string(std::strerror(errno)); return false; }
    bool okay = fchmod(fd, 0600) == 0;
    std::size_t offset = 0;
    while (okay && offset < body.size()) {
        const ssize_t count = write(fd, body.data() + offset, body.size() - offset);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) okay = false;
        else offset += static_cast<std::size_t>(count);
    }
    if (okay) okay = fsync(fd) == 0;
    if (close(fd) != 0) okay = false;
    if (okay) okay = rename(temporary.c_str(), path.c_str()) == 0;
    if (!okay) {
        unlink(temporary.c_str());
        error = "Cannot publish browser state: " + std::string(std::strerror(errno));
    }
    return okay;
}

bool restoreBrowserState(const std::string& path, const std::vector<UiRow>& rows,
                         std::size_t& selected, long& first, std::string& error) {
    error.clear();
    struct stat details {};
    if (lstat(path.c_str(), &details) != 0) {
        if (errno != ENOENT) error = "Cannot inspect browser state.";
        return false;
    }
    if (!S_ISREG(details.st_mode) || details.st_size < 0 ||
        details.st_size > static_cast<long>(limit)) {
        error = "Invalid browser state file."; return false;
    }
    std::ifstream file(path, std::ios::binary);
    std::string body(static_cast<std::size_t>(details.st_size), '\0');
    if (!body.empty()) file.read(&body[0], body.size());
    const bool complete = bool(file) && file.peek() == std::char_traits<char>::eof();
    std::istringstream input(body);
    std::string magic, launch, rom, topLaunch, topRom;
    std::size_t ordinal, oldFirst, header;
    std::getline(input, magic);
    if (!complete ||
        magic != "BetterFavoritesBrowserState1" || !number(input, ordinal) ||
        !number(input, oldFirst) || !number(input, header) || header > 1 ||
        !readField(input, launch) || !readField(input, rom) ||
        !readField(input, topLaunch) || !readField(input, topRom) ||
        input.peek() != std::char_traits<char>::eof()) {
        error = "Malformed browser state; using default position."; return false;
    }
    std::vector<std::size_t> games;
    std::size_t match = rows.size();
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (!rows[i].favorite) continue;
        games.push_back(i);
        if (match == rows.size() && rows[i].favorite->launchPath == launch &&
            rows[i].favorite->romPath == rom) match = i;
    }
    if (games.empty()) return false;
    if (match == rows.size()) match = games[std::min(ordinal, games.size() - 1)];
    std::size_t top = std::min(oldFirst, rows.size() - 1);
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if ((header && rows[i].type == UiRowType::SystemDivider &&
             !topLaunch.empty() && headerSystem(rows, i) == topLaunch) ||
            (!header && rows[i].favorite &&
             rows[i].favorite->launchPath == topLaunch && rows[i].favorite->romPath == topRom)) {
            top = i; break;
        }
    }
    selected = match;
    first = static_cast<long>(std::min(top, match));
    return true;
}
