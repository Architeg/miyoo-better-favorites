#include "favorite_removal.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#ifdef BETTER_FAVORITES_REMOVAL_TESTING
void (*favoriteRemovalTestHook)(const char*) = nullptr;
#endif
namespace {
void hook(const char* phase) {
#ifdef BETTER_FAVORITES_REMOVAL_TESTING
    if (favoriteRemovalTestHook) favoriteRemovalTestHook(phase);
#else
    (void)phase;
#endif
}
bool same(const struct stat& a, const struct stat& b) {
#ifdef __APPLE__
    const auto am = a.st_mtimespec, bm = b.st_mtimespec;
    const auto ac = a.st_ctimespec, bc = b.st_ctimespec;
#else
    const auto am = a.st_mtim, bm = b.st_mtim;
    const auto ac = a.st_ctim, bc = b.st_ctim;
#endif
    return a.st_dev == b.st_dev && a.st_ino == b.st_ino && a.st_size == b.st_size &&
        a.st_mode == b.st_mode && am.tv_sec == bm.tv_sec && am.tv_nsec == bm.tv_nsec &&
        ac.tv_sec == bc.tv_sec && ac.tv_nsec == bc.tv_nsec;
}
bool writeAll(int fd, const std::string& body, mode_t mode) {
    if (fchmod(fd, mode & 0777) != 0) return false;
    std::size_t offset = 0;
    while (offset < body.size()) {
        ssize_t n = write(fd, body.data() + offset, body.size() - offset);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return false;
        offset += static_cast<std::size_t>(n);
    }
    return fsync(fd) == 0;
}
bool matches(const std::string& path, const FavoritesSnapshot& original) {
    FavoritesSnapshot now; std::string error;
    return readFavoritesSnapshot(path, now, error) && same(now.identity, original.identity) &&
        now.bytes == original.bytes;
}
}
bool readFavoritesSnapshot(const std::string& path, FavoritesSnapshot& out, std::string& error) {
    out = {}; error.clear();
    int fd = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) { error = "Cannot read favorites: " + std::string(std::strerror(errno)); return false; }
    struct stat before {}, after {}, named {};
    bool okay = fstat(fd, &before) == 0 && S_ISREG(before.st_mode) && before.st_size >= 0 &&
        before.st_size <= 8 * 1024 * 1024;
    std::string bytes;
    char buffer[4096];
    while (okay) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n < 0 && errno == EINTR) continue;
        if (n < 0) { okay = false; break; }
        if (n == 0) break;
        bytes.append(buffer, static_cast<std::size_t>(n));
        if (bytes.size() > 8 * 1024 * 1024) okay = false;
    }
    okay = okay && fstat(fd, &after) == 0 && lstat(path.c_str(), &named) == 0 &&
        same(before, after) && same(after, named) && bytes.size() == static_cast<std::size_t>(before.st_size);
    if (close(fd) != 0) okay = false;
    if (!okay) { error = "Favorites unreadable, non-regular, too large or changed while reading."; return false; }
    out.bytes = std::move(bytes); out.identity = after; out.valid = true; return true;
}
bool removeFavorite(const std::string& path, FavoritesSnapshot& original,
                    const Favorite& favorite, std::string& backup, std::string& error) {
    error.clear(); backup.clear();
    const auto offset = favorite.sourceOffset, length = favorite.sourceRecord.size();
    if (!original.valid || !length || offset > original.bytes.size() ||
        length > original.bytes.size() - offset ||
        original.bytes.compare(offset, length, favorite.sourceRecord) != 0 ||
        (offset && original.bytes[offset - 1] != '\n') ||
        (offset + length < original.bytes.size() && favorite.sourceRecord.back() != '\n')) {
        error = "Selected favorite does not match the loaded source record."; return false;
    }
    if (!matches(path, original)) {
        error = "Favorites changed externally; reopen Better Favorites before removing."; return false;
    }
    const auto slash = path.find_last_of('/');
    const std::string directory = slash == std::string::npos ? "." : path.substr(0, slash);
    int directoryFd = open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    if (directoryFd < 0) { error = "Cannot open favorites directory."; return false; }
    backup = directory + "/.favourite.json.better-favorites-backup.XXXXXX";
    hook("before-backup");
    int backupFd = mkstemp(&backup[0]);
    bool okay = backupFd >= 0;
    if (okay) {
        okay = writeAll(backupFd, original.bytes, original.identity.st_mode);
        if (close(backupFd) != 0) okay = false;
    }
    hook("before-backup-verification");
    FavoritesSnapshot backupCopy; std::string readError;
    okay = okay && readFavoritesSnapshot(backup, backupCopy, readError) &&
        backupCopy.bytes == original.bytes && fsync(directoryFd) == 0;
    if (!okay) { close(directoryFd); error = "Could not create and verify favorites backup; source unchanged."; return false; }
    hook("after-backup");
    std::string next = original.bytes; next.erase(offset, length);
    std::string temporary = directory + "/.better-favorites-removal.XXXXXX";
    int fd = mkstemp(&temporary[0]);
    okay = fd >= 0;
    if (okay) {
        okay = writeAll(fd, next, original.identity.st_mode);
        if (close(fd) != 0) okay = false;
    }
    FavoritesSnapshot staged;
    okay = okay && readFavoritesSnapshot(temporary, staged, readError) && staged.bytes == next;
    hook("before-publication");
    if (!okay || !matches(path, original)) {
        if (fd >= 0) unlink(temporary.c_str());
        close(directoryFd);
        error = okay ? "Favorites changed externally; removal cancelled." : "Cannot stage removal; source unchanged.";
        return false;
    }
    // Onion serializes the app with MainUI. No POSIX compare-and-swap rename:
    // an uncooperative external writer in the final check/rename gap remains a risk.
    if (rename(temporary.c_str(), path.c_str()) != 0) {
        const bool cleaned = unlink(temporary.c_str()) == 0;
        close(directoryFd);
        error = "Cannot publish removal; source unchanged.";
        if (!cleaned) error += " Temporary file retained: " + temporary;
        return false;
    }
    // Once committed, never roll back over a possible later external writer.
    const bool synced = fsync(directoryFd) == 0;
    close(directoryFd);
    original = {};
    if (!synced) error = "Removed, but directory sync failed; verified backup retained.";
    return true;
}
