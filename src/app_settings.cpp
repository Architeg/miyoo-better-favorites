#include "settings.h"
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr const char* magic = "BetterFavoritesSettings1";
bool generation(const std::string& value) {
    if (value.size() != 32) return false;
    for (char c : value) if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
    return true;
}
}

bool automaticReturnAvailable() {
    // A live private context proves this invocation came through the hook.
    // A saved ON preference or a helper file on disk does not prove availability.
    const char* value = std::getenv("BETTER_FAVORITES_RETURN_DIR");
    if (!value) return false;
    const std::string path = value;
    const std::string prefix = "/tmp/better-favorites-return.";
    struct stat details {};
    return path.compare(0, prefix.size(), prefix) == 0 && path.size() > prefix.size() &&
        path.find('/', prefix.size()) == std::string::npos &&
        lstat(path.c_str(), &details) == 0 && S_ISDIR(details.st_mode) &&
        details.st_uid == geteuid() && (details.st_mode & 077) == 0;
}

bool loadAppSettings(const std::string& path, AppSettings& settings, std::string& error) {
    settings.automaticReturn = false;
    settings.returnGeneration.clear();
    error.clear();
    struct stat details {};
    if (lstat(path.c_str(), &details) != 0) {
        if (errno == ENOENT) return true;
        error = "Cannot inspect app settings."; return false;
    }
    if (!S_ISREG(details.st_mode) || details.st_size < 0 || details.st_size > 128) {
        error = "Invalid app settings; automatic return is off."; return false;
    }
    std::ifstream input(path, std::ios::binary);
    std::string body(static_cast<std::size_t>(details.st_size), '\0');
    if (!body.empty()) input.read(&body[0], body.size());
    std::istringstream record(body);
    std::string header, enabled, epoch;
    std::getline(record, header); std::getline(record, enabled); std::getline(record, epoch);
    const std::string expected = std::string(magic) + "\n" + enabled + "\n" + epoch + "\n";
    if (!input || input.peek() != std::char_traits<char>::eof() || body != expected ||
        header != magic || (enabled != "0" && enabled != "1") || !generation(epoch)) {
        error = "Malformed app settings; automatic return is off."; return false;
    }
    settings.automaticReturn = enabled == "1";
    settings.returnGeneration = epoch;
    return true;
}

bool setAutomaticReturn(const std::string& path, bool enabled, AppSettings& settings,
                        std::string& error) {
    error.clear();
    struct stat details {};
    if (lstat(path.c_str(), &details) == 0) {
        if (!S_ISREG(details.st_mode)) { error = "Settings path is not a regular file."; return false; }
    } else if (errno != ENOENT) { error = "Cannot inspect settings path."; return false; }
    unsigned char random[16];
    const int source = open("/dev/urandom", O_RDONLY);
    if (source < 0) { error = "Cannot create settings generation."; return false; }
    std::size_t offset = 0;
    while (offset < sizeof(random)) {
        const ssize_t count = read(source, random + offset, sizeof(random) - offset);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) break;
        offset += static_cast<std::size_t>(count);
    }
    const bool closed = close(source) == 0;
    if (offset != sizeof(random) || !closed) { error = "Cannot read settings generation."; return false; }
    const char* hex = "0123456789abcdef";
    std::string epoch;
    for (unsigned char c : random) { epoch += hex[c >> 4]; epoch += hex[c & 15]; }
    const std::string body = std::string(magic) + "\n" + (enabled ? "1\n" : "0\n") + epoch + "\n";
    std::string temporary = path + ".XXXXXX";
    const int fd = mkstemp(&temporary[0]);
    if (fd < 0) { error = "Cannot create app settings."; return false; }
    bool okay = fchmod(fd, 0600) == 0;
    offset = 0;
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
        error = "Cannot publish app settings: " + std::string(std::strerror(errno));
        return false;
    }
    settings.automaticReturn = enabled;
    settings.returnGeneration = epoch;
    return true;
}

// Installer-owned marker, never a switch or return generation. No font or
// browser state is changed by this one-time informational page.
bool pendingWelcome(const std::string& path) {
 const int fd=open(path.c_str(),O_RDONLY|O_NONBLOCK|O_NOFOLLOW);
 if(fd<0)return false;
 struct stat s{};char buffer[25]{};
 const bool regular=fstat(fd,&s)==0 && S_ISREG(s.st_mode) && s.st_size==24;
 const auto count=regular?read(fd,buffer,sizeof(buffer)):-1;close(fd);
 return count==24 && std::string(buffer,24)=="BetterFavoritesWelcome1\n";
}
bool dismissWelcome(const std::string& path,std::string& error) {
 if(!pendingWelcome(path)){error="First-launch notice changed; it was kept.";return false;}
 if(unlink(path.c_str())!=0){error="Cannot save first-launch acknowledgement.";return false;}
 error.clear();return true;
}
