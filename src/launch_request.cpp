#include "launch_request.h"
#include "settings.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <limits.h>
#include <fstream>
#include <iterator>
#include <string>
#include <iostream>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace
{

constexpr const char* kSdRoot = "/mnt/SDCARD";
constexpr const char* kActiveCommand =
    "/mnt/SDCARD/.tmp_update/cmd_to_run.sh";
constexpr const char* kQuickSwitch = "/tmp/quick_switch";

bool startsWith(const std::string& value, const std::string& prefix)
{
    return value.compare(0, prefix.size(), prefix) == 0;
}

bool safePathCharacters(const std::string& path)
{
    // The mounted runtime parses the command with awk, sed, and a shell.
    // Reject expansion, quoting, control, and parser characters. UTF-8
    // bytes are retained; they cannot be ASCII shell syntax bytes.
    static const std::string punctuation = " /._-+,()[]'@#%";

    for (unsigned char c : path) {
        if (
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c >= 0x80 ||
            punctuation.find(static_cast<char>(c)) !=
                std::string::npos
        ) {
            continue;
        }

        return false;
    }

    return true;
}

bool regularFile(const std::string& path, std::string& error)
{
    struct stat details {};

    if (stat(path.c_str(), &details) != 0 ||
        !S_ISREG(details.st_mode)) {
        error = "Missing or unsupported file: " + path;
        return false;
    }

    return true;
}

bool writeAll(int fd, const std::string& command)
{
    std::size_t offset = 0;

    while (offset < command.size()) {
        const ssize_t count = write(
            fd,
            command.data() + offset,
            command.size() - offset
        );

        if (count < 0 && errno == EINTR) {
            continue;
        }

        if (count <= 0) {
            if (count == 0) {
                errno = EIO;
            }
            return false;
        }

        offset += static_cast<std::size_t>(count);
    }

    return true;
}

bool validRequestDirectory(
    const std::string& directory,
    std::string& error
)
{
    static const std::string prefix = "/tmp/better-favorites.";
    struct stat details {};

    if (
        !startsWith(directory, prefix) ||
        directory.size() == prefix.size() ||
        directory.find('/', prefix.size()) != std::string::npos ||
        lstat(directory.c_str(), &details) != 0 ||
        !S_ISDIR(details.st_mode) ||
        details.st_uid != geteuid() ||
        (details.st_mode & 077) != 0
    ) {
        error = "Invalid private launch request directory.";
        return false;
    }

    return true;
}

std::string stagedPath(const std::string& directory)
{
    return directory + "/request.sh";
}

std::string appCommandPath(const std::string& directory)
{
    return directory + "/app-command.sh";
}

std::string recentRecordPath(const std::string& directory)
{
    return directory + "/recent.json";
}

std::string jsonString(const std::string& value)
{
    static const char hex[] = "0123456789abcdef";
    std::string result = "\"";
    for (unsigned char c : value) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (c < 0x20) {
                result += "\\u00";
                result += hex[c >> 4];
                result += hex[c & 15];
            } else {
                result += static_cast<char>(c);
            }
        }
    }
    return result + '"';
}

bool parserPath(const std::string& value)
{
    // v4.3.1-1 uses strstr/strchr rather than JSON string decoding and
    // resumeGame() copies each extracted field into a 256-byte buffer.
    if (value.empty() || value.size() >= 256) return false;
    for (unsigned char c : value) {
        if (c < 0x20 || c == '"' || c == '\\') return false;
    }
    return true;
}

bool readCommand(
    const std::string& path,
    std::string& contents,
    std::string& error
)
{
    struct stat details {};
    if (lstat(path.c_str(), &details) != 0 ||
        !S_ISREG(details.st_mode) || details.st_size > 4096) {
        error = "Missing or invalid command file: " + path;
        return false;
    }
    std::ifstream input(path, std::ios::binary);
    contents.assign(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    );
    if (input.bad() || contents.empty() ||
        contents.size() != static_cast<std::size_t>(details.st_size)) {
        error = "Cannot read command file: " + path;
        return false;
    }
    return true;
}

bool validAppCommand(const std::string& command)
{
    // This is the app command observed on this card. Only trailing shell
    // whitespace varies with runtime command-file generation.
    static const std::string expected =
        "cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; "
        "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh";
    if (!startsWith(command, expected)) {
        return false;
    }
    for (std::size_t i = expected.size(); i < command.size(); ++i) {
        if (command[i] != ' ' && command[i] != '\t' &&
            command[i] != '\n') {
            return false;
        }
    }
    return true;
}

bool writeTemporary(
    const std::string& destination,
    const std::string& contents,
    std::string& temporary,
    std::string& error,
    mode_t mode = 0700
)
{
    std::string pattern = destination + ".better-favorites.XXXXXX";
    std::string buffer = pattern;
    const int fd = mkstemp(&buffer[0]);
    if (fd < 0) {
        error = "Cannot create destination temporary file: " +
            std::string(std::strerror(errno));
        return false;
    }
    temporary = buffer;
    bool okay = fchmod(fd, mode) == 0 &&
        writeAll(fd, contents) && fsync(fd) == 0;
    int savedError = okay ? 0 : errno;
    if (close(fd) != 0) {
        okay = false;
        savedError = errno;
    }
    if (!okay) {
        unlink(temporary.c_str());
        temporary.clear();
        error = "Cannot write destination temporary file: " +
            std::string(std::strerror(savedError));
    }
    return okay;
}

struct FileSnapshot {
    bool exists = false;
    struct stat details {};
    std::string contents;
};

bool readSnapshot(
    const std::string& path,
    FileSnapshot& snapshot,
    std::string& error
)
{
    snapshot = {};
    const int fd = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd < 0) {
        if (errno == ENOENT) return true;
        error = "Cannot read " + path + ": " + std::strerror(errno);
        return false;
    }
    snapshot.exists = true;
    bool okay = fstat(fd, &snapshot.details) == 0 &&
        S_ISREG(snapshot.details.st_mode) &&
        snapshot.details.st_size <= 16 * 1024 * 1024;
    char buffer[4096];
    while (okay) {
        const ssize_t count = read(fd, buffer, sizeof(buffer));
        if (count < 0 && errno == EINTR) continue;
        if (count < 0) { okay = false; break; }
        if (count == 0) break;
        snapshot.contents.append(buffer, static_cast<std::size_t>(count));
        if (snapshot.contents.size() > 16 * 1024 * 1024) okay = false;
    }
    struct stat after {};
    okay = okay && fstat(fd, &after) == 0 &&
        after.st_size == snapshot.details.st_size &&
        snapshot.contents.size() ==
            static_cast<std::size_t>(snapshot.details.st_size);
    if (close(fd) != 0) okay = false;
    if (!okay) error = "Invalid or changing file: " + path;
    return okay;
}

bool sameSnapshot(const FileSnapshot& a, const FileSnapshot& b)
{
    return a.exists == b.exists && (!a.exists ||
        (a.details.st_dev == b.details.st_dev &&
         a.details.st_ino == b.details.st_ino &&
         a.contents == b.contents));
}

struct Replacement {
    std::string path;
    FileSnapshot before;
    std::string contents;
    std::string temporary;
    std::string backup;
    struct stat publishedDetails {};
    bool published = false;

    ~Replacement()
    {
        if (!temporary.empty()) unlink(temporary.c_str());
        if (!backup.empty()) unlink(backup.c_str());
    }

    bool prepare(std::string& error, mode_t mode)
    {
        if (!writeTemporary(path, contents, temporary, error, mode))
            return false;
        if (lstat(temporary.c_str(), &publishedDetails) != 0) {
            error = "Cannot inspect temporary replacement: " + path;
            return false;
        }
        return !before.exists || writeTemporary(
            path, before.contents, backup, error,
            before.details.st_mode & 0777);
    }

    bool publish(std::string& error)
    {
        FileSnapshot current;
        if (!readSnapshot(path, current, error)) return false;
        if (!sameSnapshot(before, current)) {
            error = "File changed before publication: " + path;
            return false;
        }
        // Onion has no shared lock or compare-and-rename operation. The
        // snapshot check and rename still assume no writer in this interval.
        if (rename(temporary.c_str(), path.c_str()) != 0) {
            error = "Cannot publish " + path + ": " + std::strerror(errno);
            return false;
        }
        temporary.clear();
        published = true;
        return true;
    }

    bool stillPublished(std::string& error)
    {
        FileSnapshot expected;
        expected.exists = true;
        expected.details = publishedDetails;
        expected.contents = contents;
        FileSnapshot current;
        if (!readSnapshot(path, current, error)) return false;
        if (sameSnapshot(expected, current)) return true;
        error = "Published file changed before quick-switch flag: " + path;
        return false;
    }

    bool rollback(std::string& error, const std::string& ownedPrefix = "")
    {
        if (!published) return true;
        FileSnapshot current;
        if (!readSnapshot(path, current, error)) return false;
        const bool ownedFile = current.exists &&
            current.details.st_dev == publishedDetails.st_dev &&
            current.details.st_ino == publishedDetails.st_ino;
        if (!current.exists) return true;

        if (!ownedFile || current.contents != contents) {
            if (ownedPrefix.empty()) {
                error = "Unrelated command left unchanged: " + path;
                return true;
            }
            // An independent append can keep our inode; an atomic prepend
            // replaces it. Remove only the uniquely identifiable insertion,
            // preserving the other writer's prefix/suffix. If a rewrite
            // destroys that context, do not guess which duplicate is ours.
            if (current.contents == before.contents) return true;
            std::size_t insertion = std::string::npos;
            const std::size_t block = current.contents.find(contents);
            if (block != std::string::npos &&
                (block == 0 || current.contents[block - 1] == '\n') &&
                current.contents.find(contents, block + 1) ==
                    std::string::npos) insertion = block;
            if (insertion == std::string::npos) {
                if (current.contents.find(ownedPrefix) == std::string::npos)
                    return true; // Our record was already removed.
                error = "Cannot identify owned recent record after history rewrite.";
                return false;
            }
            std::string remainder = current.contents;
            remainder.erase(insertion, ownedPrefix.size());
            std::string corrected;
            if (!writeTemporary(path, remainder, corrected, error,
                                current.details.st_mode & 0777)) return false;
            FileSnapshot check;
            if (!readSnapshot(path, check, error) ||
                !sameSnapshot(current, check)) {
                unlink(corrected.c_str());
                if (error.empty()) error = "History changed during rollback.";
                return false;
            }
            const bool restored = rename(corrected.c_str(), path.c_str()) == 0;
            if (!restored) {
                unlink(corrected.c_str());
                error = "Cannot remove owned history record: " +
                    std::string(std::strerror(errno));
            }
            return restored;
        }
        if (before.exists) {
            if (rename(backup.c_str(), path.c_str()) == 0) {
                backup.clear();
                return true;
            }
            error = "Cannot restore " + path + ": " + std::strerror(errno);
            // If restoring the command fails, remove our queued game only.
            if (ownedPrefix.empty()) {
                FileSnapshot check;
                std::string checkError;
                if (readSnapshot(path, check, checkError) &&
                    sameSnapshot(current, check) && unlink(path.c_str()) != 0)
                    error += "; could not remove owned game command";
            }
            return false;
        }
        if (unlink(path.c_str()) == 0) return true;
        error = "Cannot remove owned file: " + path;
        return false;
    }
};

#ifdef BETTER_FAVORITES_HANDOFF_TESTING
void (*handoffTestHook)(const char*) = nullptr;
#endif

void handoffTestPoint(const char* phase)
{
#ifdef BETTER_FAVORITES_HANDOFF_TESTING
    if (handoffTestHook) handoffTestHook(phase);
#else
    (void)phase;
#endif
}

}

#ifdef BETTER_FAVORITES_HANDOFF_TESTING
void setOnionHandoffTestHook(void (*hook)(const char* phase))
{
    handoffTestHook = hook;
}
#endif

bool buildOnionLaunchCommand(
    const Favorite& favorite,
    std::string& command,
    std::string& error
)
{
    command.clear();
    error.clear();

    const std::string& launch = favorite.launchPath;
    const std::string& rom = favorite.romPath;
    const std::string emuPrefix = std::string(kSdRoot) + "/Emu/";

    if (launch.empty() || rom.empty()) {
        error = "Selected favorite has no launch or ROM path.";
        return false;
    }

    if (!safePathCharacters(launch) || !safePathCharacters(rom)) {
        error = "Launch or ROM path contains unsupported characters.";
        return false;
    }

    if (!startsWith(launch, emuPrefix)) {
        error = "Launch path is outside Onion's Emu directory.";
        return false;
    }

    const std::size_t systemEnd = launch.find('/', emuPrefix.size());

    if (
        systemEnd == std::string::npos ||
        systemEnd == emuPrefix.size() ||
        launch.substr(systemEnd) != "/launch.sh"
    ) {
        error = "Only Onion Emu/<system>/launch.sh is supported.";
        return false;
    }

    const std::string system = launch.substr(
        emuPrefix.size(),
        systemEnd - emuPrefix.size()
    );

    if (
        system == "." || system == ".." ||
        system.find(' ') != std::string::npos
    ) {
        error = "Unsupported emulator directory name.";
        return false;
    }

    const std::string directRomPrefix = std::string(kSdRoot) + "/Roms/";
    const std::string relativeRomPrefix =
        emuPrefix + system + "/../../Roms/";

    if (
        !startsWith(rom, directRomPrefix) &&
        !startsWith(rom, relativeRomPrefix)
    ) {
        error = "ROM path does not match Onion's game path format.";
        return false;
    }

    // Onion stores paths containing /../../ and /./. Filesystem validation
    // checks their resolved targets; command generation preserves spelling.
    command =
        "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so \"" +
        launch + "\" \"" + rom + "\"\n";

    return true;
}

bool validateOnionLaunchFiles(
    const Favorite& favorite,
    const std::string& sdRoot,
    std::string& error
)
{
    error.clear();
    const std::string storedPrefix = std::string(kSdRoot) + "/";
    if (!startsWith(favorite.launchPath, storedPrefix) ||
        !startsWith(favorite.romPath, storedPrefix)) {
        error = "Launch or ROM path is outside the SD card.";
        return false;
    }

    const std::string launch = sdRoot +
        favorite.launchPath.substr(std::strlen(kSdRoot));
    const std::string rom = sdRoot +
        favorite.romPath.substr(std::strlen(kSdRoot));
    char resolvedRoot[PATH_MAX];
    char resolvedLaunch[PATH_MAX];
    char resolvedRom[PATH_MAX];
    if (!realpath(sdRoot.c_str(), resolvedRoot) ||
        !realpath(launch.c_str(), resolvedLaunch) ||
        !realpath(rom.c_str(), resolvedRom)) {
        error = "Cannot resolve selected launch or ROM path: " +
            std::string(std::strerror(errno));
        return false;
    }

    const std::string root = resolvedRoot;
    if (!startsWith(resolvedLaunch, root + "/Emu/") ||
        !startsWith(resolvedRom, root + "/Roms/")) {
        error = "Resolved launch or ROM path is outside Onion's directories.";
        return false;
    }
    // Runtime may rewrite the ROM argument using realpath before executing
    // the command. A safe stored alias must not resolve to unsafe shell text.
    if (!safePathCharacters(resolvedLaunch) ||
        !safePathCharacters(resolvedRom)) {
        error = "Resolved launch or ROM path contains unsupported characters.";
        return false;
    }
    if (!regularFile(resolvedLaunch, error) ||
        !regularFile(resolvedRom, error)) {
        return false;
    }
    if (access(resolvedLaunch, X_OK) != 0) {
        error = "Onion launch script is not executable.";
        return false;
    }
    return true;
}

bool buildOnionRecentRecord(
    const Favorite& favorite,
    const std::string& sdRoot,
    std::string& record,
    std::string& error
)
{
    record.clear();
    std::string command;
    if (!buildOnionLaunchCommand(favorite, command, error)) return false;
    bool imageExists = false;
    if (!favorite.imagePath.empty()) {
        std::string image = favorite.imagePath;
        if (startsWith(image, std::string(kSdRoot) + "/"))
            image = sdRoot + image.substr(std::strlen(kSdRoot));
        struct stat imageDetails {};
        if (stat(image.c_str(), &imageDetails) == 0) {
            imageExists = true; // Match RandomGamePicker's test -e.
        } else if (errno != ENOENT && errno != ENOTDIR) {
            error = "Cannot inspect favorite image: " +
                std::string(std::strerror(errno));
            return false;
        }
    }
    if (!parserPath(favorite.romPath) ||
        !parserPath(favorite.launchPath) ||
        (imageExists && (!parserPath(favorite.imagePath) ||
                         favorite.imagePath.front() != '/'))) {
        error = "Recent-list path is incompatible with Onion's string parser.";
        return false;
    }
    const std::string label = jsonString(favorite.label);
    if (favorite.label.empty() || label.size() - 2 >= 256) {
        error = "Recent-list label is empty or too long for Onion.";
        return false;
    }

    // Keep field markers compact and do not escape path slashes. Onion's
    // string parser does not decode JSON escapes in paths. Escaped label
    // text is valid JSON; GS derives the displayed title from the ROM/cache,
    // not its extracted label (which is only used for debug logging).
    record = "{\"label\":" + label +
        ",\"rompath\":" + jsonString(favorite.romPath) +
        ",\"launch\":" + jsonString(favorite.launchPath);
    if (imageExists)
        record += ",\"imgpath\":" + jsonString(favorite.imagePath);
    record += ",\"type\":5}\n";
    // readHistory() has a 3 * STR_MAX (256) line buffer in v4.3.1-1.
    if (record.size() >= 768) {
        record.clear();
        error = "Recent-list record exceeds Onion's line buffer.";
        return false;
    }
    return true;
}

namespace
{

bool writePrivateFile(
    const std::string& path,
    const std::string& contents,
    std::string& error
)
{
    const int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        error = "Cannot stage " + path + ": " + std::strerror(errno);
        return false;
    }
    bool okay = writeAll(fd, contents) && fsync(fd) == 0;
    int savedError = okay ? 0 : errno;
    if (close(fd) != 0) { okay = false; savedError = errno; }
    if (!okay) {
        unlink(path.c_str());
        error = "Cannot stage " + path + ": " + std::strerror(savedError);
    }
    return okay;
}

}

bool stageOnionLaunchCommand(
    const std::string& command,
    const std::string& recentRecord,
    const std::string& requestDir,
    std::string& error
)
{
    error.clear();

    if (!validRequestDirectory(requestDir, error)) {
        return false;
    }

    if (!writePrivateFile(stagedPath(requestDir), command, error))
        return false;
    if (!writePrivateFile(recentRecordPath(requestDir), recentRecord, error)) {
        unlink(stagedPath(requestDir).c_str());
        return false;
    }

    return true;
}

bool cancelOnionLaunchCommand(
    const std::string& requestDir,
    std::string& error
)
{
    error.clear();

    if (!validRequestDirectory(requestDir, error)) {
        return false;
    }

    for (const std::string& path : {
             stagedPath(requestDir), appCommandPath(requestDir),
             recentRecordPath(requestDir), requestDir + "/switcher.request"}) {
        struct stat details {};
        if (lstat(path.c_str(), &details) != 0) {
            if (errno == ENOENT) continue;
            error = "Cannot inspect private request file: " + path;
            return false;
        }
        if (!S_ISREG(details.st_mode) || details.st_uid != geteuid()) {
            error = "Private request file has unexpected ownership or type.";
            return false;
        }
        if (unlink(path.c_str()) != 0) {
            error = "Cannot remove private request file: " +
                std::string(std::strerror(errno));
            return false;
        }
    }
    return true;
}

bool publishOnionLaunchCommand(
    const std::string& requestDir,
    const std::string& activePath,
    const std::string& quickSwitchPath,
    const std::string& sdRoot,
    std::string& error
)
{
    error.clear();
    if (!validRequestDirectory(requestDir, error)) return false;

    Replacement command;
    command.path = activePath;
    std::string appCommand;
    std::string recentRecord;
    if (!readCommand(appCommandPath(requestDir), appCommand, error) ||
        !validAppCommand(appCommand) ||
        !readSnapshot(activePath, command.before, error) ||
        !command.before.exists || command.before.contents != appCommand) {
        if (error.empty())
            error = "Active Onion command is not this app invocation.";
        return false;
    }
    if (!readCommand(stagedPath(requestDir), command.contents, error) ||
        !readCommand(recentRecordPath(requestDir), recentRecord, error))
        return false;

    struct stat flagDetails {};
    if (lstat(quickSwitchPath.c_str(), &flagDetails) == 0) {
        error = "Quick-switch flag already exists.";
        return false;
    }
    if (errno != ENOENT) {
        error = "Cannot inspect quick-switch flag.";
        return false;
    }

    // Match the mounted RandomGamePicker's .showRecents selection.
    const std::string showRecents = sdRoot + "/.tmp_update/config/.showRecents";
    struct stat showDetails {};
    const bool showExists = stat(showRecents.c_str(), &showDetails) == 0;
    if (!showExists && errno != ENOENT) {
        error = "Cannot inspect Onion's Recents setting.";
        return false;
    }
    Replacement history;
    history.path = sdRoot + "/Roms/" +
        (showExists && S_ISREG(showDetails.st_mode)
            ? "recentlist.json" : "recentlist-hidden.json");
    if (!readSnapshot(history.path, history.before, error)) return false;
    history.contents = recentRecord + history.before.contents;
    const mode_t historyMode = history.before.exists
        ? history.before.details.st_mode & 0777 : 0600;

    // Only the patched runtime exports a fresh, invocation-private context.
    // Stock runtime installs retain the existing launch behavior.
    Replacement origin;
    const char* returnDir = std::getenv("BETTER_FAVORITES_RETURN_DIR");
    AppSettings appSettings;
    const char* settingsEnvironment = std::getenv("BETTER_FAVORITES_SETTINGS");
    const std::string settingsPath = settingsEnvironment && *settingsEnvironment
        ? settingsEnvironment : sdRoot + "/App/BetterFavoritesTest/settings.conf";
    std::string settingsError;
    loadAppSettings(settingsPath, appSettings, settingsError);
    if (!settingsError.empty()) std::cerr << settingsError << '\n';
    const bool registerOrigin = returnDir && *returnDir && appSettings.automaticReturn;
    Replacement originGeneration;
    if (registerOrigin) {
        const std::string directory = returnDir;
        const std::string prefix = "/tmp/better-favorites-return.";
        struct stat details {};
        if (!startsWith(directory, prefix) || directory.size() == prefix.size() ||
            directory.find('/', prefix.size()) != std::string::npos ||
            lstat(directory.c_str(), &details) != 0 ||
            !S_ISDIR(details.st_mode) || details.st_uid != geteuid() ||
            (details.st_mode & 077) != 0) {
            error = "Invalid runtime return context.";
            return false;
        }
        origin.path = directory + "/request.sh";
        origin.contents = command.contents;
        if (!readSnapshot(origin.path, origin.before, error)) return false;
        if (origin.before.exists) {
            error = "Runtime return context already contains a ticket.";
            return false;
        }
        originGeneration.path = directory + "/generation";
        originGeneration.contents = appSettings.returnGeneration + "\n";
        if (!readSnapshot(originGeneration.path, originGeneration.before, error)) return false;
        if (originGeneration.before.exists) {
            error = "Runtime return context already contains a generation."; return false;
        }
        if (!origin.prepare(error, 0600) || !originGeneration.prepare(error, 0600)) return false;
    }

    // Prepare all files on their destination filesystem before mutating
    // either active path. Existing recent records remain byte-for-byte.
    if (!command.prepare(error, 0700) ||
        !history.prepare(error, historyMode)) return false;

    auto rollback = [&]() {
        std::string historyError;
        std::string commandError;
        std::string originError;
        std::string generationError;
        const bool generationOkay = originGeneration.rollback(generationError);
        const bool originOkay = origin.rollback(originError);
        const bool historyOkay = history.rollback(historyError, recentRecord);
        const bool commandOkay = command.rollback(commandError);
        if (!historyError.empty()) error += "; history rollback: " + historyError;
        if (!commandError.empty()) error += "; command rollback: " + commandError;
        if (!originError.empty()) error += "; origin rollback: " + originError;
        if (!generationError.empty()) error += "; generation rollback: " + generationError;
        if (historyOkay && commandOkay && originOkay && generationOkay)
            std::cerr << "Handoff rollback completed; unrelated changes preserved.\n";
    };

    if (!command.publish(error)) return false;
    handoffTestPoint("after-command");
    if (!history.publish(error)) {
        rollback();
        return false;
    }
    handoffTestPoint("after-history");

    if (registerOrigin && (!origin.publish(error) || !originGeneration.publish(error))) {
        rollback();
        return false;
    }
    if (!command.stillPublished(error) || !history.stillPublished(error) ||
        (registerOrigin && (!origin.stillPublished(error) || !originGeneration.stillPublished(error)))) {
        rollback();
        return false;
    }

    // Runtime will observe the command/history only after this app returns.
    // Set quick_switch last so a partially published request is not queued.
    const int flag = open(
        quickSwitchPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (flag >= 0) {
        struct stat createdFlagDetails {};
        const bool identified = fstat(flag, &createdFlagDetails) == 0;
        const bool synced = fsync(flag) == 0;
        const bool closed = close(flag) == 0;
        if (identified && synced && closed) {
            std::cerr << "Registered Onion recent-list entry: "
                      << history.path << '\n';
            return true;
        }
        error = "Cannot verify, sync, or close quick-switch flag.";
        struct stat currentFlagDetails {};
        if (identified &&
            lstat(quickSwitchPath.c_str(), &currentFlagDetails) == 0 &&
            currentFlagDetails.st_dev == createdFlagDetails.st_dev &&
            currentFlagDetails.st_ino == createdFlagDetails.st_ino) {
            if (unlink(quickSwitchPath.c_str()) != 0)
                error += "; could not remove owned quick-switch flag";
        }
    } else {
        error = "Cannot set quick-switch flag: " +
            std::string(std::strerror(errno));
    }
    rollback();
    return false;
}

bool publishStagedOnionLaunch(
    const std::string& requestDir,
    std::string& error
)
{
    return publishOnionLaunchCommand(
        requestDir,
        kActiveCommand,
        kQuickSwitch,
        kSdRoot,
        error
    );
}

bool cancelStagedOnionLaunch(
    const std::string& requestDir,
    std::string& error
)
{
    return cancelOnionLaunchCommand(
        requestDir,
        error
    );
}

bool requestOnionLaunch(
    const Favorite& favorite,
    std::string& error
)
{
    error.clear();

    const char* requestDir =
        std::getenv("BETTER_FAVORITES_REQUEST_DIR");

    if (
        !requestDir ||
        !validRequestDirectory(requestDir, error)
    ) {
        if (error.empty()) {
            error = "Private launch directory is unavailable.";
        }
        return false;
    }

    std::string capturedAppCommand;
    std::string activeAppCommand;
    if (!readCommand(appCommandPath(requestDir),
                     capturedAppCommand, error) ||
        !validAppCommand(capturedAppCommand) ||
        !readCommand(kActiveCommand, activeAppCommand, error) ||
        activeAppCommand != capturedAppCommand) {
        if (error.empty())
            error = "Active Onion command is not this app invocation.";
        return false;
    }

    std::string command;

    if (!buildOnionLaunchCommand(favorite, command, error)) {
        return false;
    }

    if (!validateOnionLaunchFiles(favorite, kSdRoot, error)) {
        return false;
    }

    std::string recentRecord;
    if (!buildOnionRecentRecord(favorite, kSdRoot, recentRecord, error))
        return false;

    return stageOnionLaunchCommand(
        command,
        recentRecord,
        requestDir,
        error
    );
}

namespace {
constexpr const char* kSwitcherRequest = "BetterFavoritesSwitcher1\n";
bool menuPathAbsent(const std::string& path, std::string& error) {
    struct stat details {};
    if (lstat(path.c_str(), &details) == 0) {
        error = "Existing Onion handoff file: " + path; return false;
    }
    if (errno == ENOENT) return true;
    error = "Cannot inspect Onion handoff file: " + path; return false;
}
}

bool stageOnionSwitcherRequest(const std::string& requestDir, std::string& error) {
    error.clear();
    return validRequestDirectory(requestDir, error) &&
        writePrivateFile(requestDir + "/switcher.request", kSwitcherRequest, error);
}

bool publishOnionSwitcherRequest(const std::string& requestDir,
    const std::string& activePath, const std::string& quickSwitchPath,
    const std::string& pendingPath, const std::string& shutdownPath,
    const std::string& sdRoot, std::string& error) {
    error.clear();
    if (!validRequestDirectory(requestDir, error)) return false;
    std::string marker, captured;
    FileSnapshot active;
    if (!readCommand(requestDir + "/switcher.request", marker, error) ||
        marker != kSwitcherRequest ||
        !readCommand(appCommandPath(requestDir), captured, error) ||
        !validAppCommand(captured) || !readSnapshot(activePath, active, error) ||
        !active.exists || active.contents != captured) {
        if (error.empty()) error = "MENU request or active app ownership is invalid.";
        return false;
    }
    const std::string flagPath = sdRoot + "/.tmp_update/.runGameSwitcher";
    const std::string flagContents = std::string(kSwitcherRequest) + requestDir + "\n";
    if (!menuPathAbsent(flagPath, error) || !menuPathAbsent(quickSwitchPath, error) ||
        !menuPathAbsent(pendingPath, error) || !menuPathAbsent(shutdownPath, error)) return false;
    const std::string switcher = sdRoot + "/.tmp_update/bin/gameSwitcher";
    if (!regularFile(switcher, error) || access(switcher.c_str(), X_OK) != 0) {
        error = "Onion GameSwitcher is unavailable."; return false;
    }
    AppSettings settings;
    const char* settingsEnv = std::getenv("BETTER_FAVORITES_SETTINGS");
    std::string settingsError;
    loadAppSettings(settingsEnv && *settingsEnv ? settingsEnv :
        sdRoot + "/App/BetterFavoritesTest/settings.conf", settings, settingsError);
    if (!settingsError.empty()) std::cerr << settingsError << '\n';
    Replacement ticket, generation, restoreApp;
    if (settings.automaticReturn) {
        const char* capability = std::getenv("BETTER_FAVORITES_SWITCHER_HANDOFF");
        const char* context = std::getenv("BETTER_FAVORITES_RETURN_DIR");
        struct stat details {};
        const std::string directory = context ? context : "";
        const std::string prefix = "/tmp/better-favorites-return.";
        if (!capability || std::string(capability) != "1" ||
            !startsWith(directory, prefix) || directory.size() == prefix.size() ||
            directory.find('/', prefix.size()) != std::string::npos ||
            lstat(directory.c_str(), &details) != 0 || !S_ISDIR(details.st_mode) ||
            details.st_uid != geteuid() || (details.st_mode & 077) != 0) {
            error = "Automatic return requires the MENU-capable runtime helper."; return false;
        }
        ticket.path = directory + "/switcher.request"; ticket.contents = flagContents;
        generation.path = directory + "/generation";
        generation.contents = settings.returnGeneration + "\n";
        if (!readSnapshot(ticket.path, ticket.before, error) || ticket.before.exists ||
            !readSnapshot(generation.path, generation.before, error) || generation.before.exists) {
            if (error.empty()) error = "Return context is already occupied.";
            return false;
        }
        if (!ticket.prepare(error, 0600) || !generation.prepare(error, 0600)) return false;
    }
    restoreApp.path = activePath; restoreApp.contents = captured;
    if (!restoreApp.prepare(error, active.details.st_mode & 0777)) return false;
    bool removed = false, flagOwned = false;
    struct stat ownedFlag {};
    auto rollback = [&]() {
        if (flagOwned) {
            FileSnapshot flag;
            std::string problem;
            if (readSnapshot(flagPath, flag, problem) && flag.exists &&
                flag.details.st_dev == ownedFlag.st_dev && flag.details.st_ino == ownedFlag.st_ino &&
                flag.contents.size() <= flagContents.size() &&
                flagContents.compare(0, flag.contents.size(), flag.contents) == 0) {
                // An interrupted/short write is still our inode and token prefix.
                // A foreign rewrite with other contents remains untouched.
                if (unlink(flagPath.c_str()) != 0) error += "; cannot remove owned GameSwitcher flag";
            }
        }
        std::string problem;
        generation.rollback(problem); if (!problem.empty()) error += "; " + problem;
        problem.clear(); ticket.rollback(problem); if (!problem.empty()) error += "; " + problem;
        if (removed) {
            // The saved command is restored only into an absent slot. This is
            // a conflict guard, not CAS; Onion's app-return writer is serialized.
            problem.clear();
            if (!restoreApp.publish(problem)) error += "; app restore: " + problem;
        }
    };
    if (settings.automaticReturn && (!ticket.publish(error) || !generation.publish(error))) {
        rollback(); return false;
    }
    handoffTestPoint("menu-before-remove");
    FileSnapshot current;
    if (!readSnapshot(activePath, current, error) || !sameSnapshot(active, current) ||
        !menuPathAbsent(quickSwitchPath, error) || !menuPathAbsent(pendingPath, error) ||
        !menuPathAbsent(shutdownPath, error)) {
        if (error.empty()) error = "Active app command changed before MENU handoff.";
        rollback(); return false;
    }
    if (unlink(activePath.c_str()) != 0) {
        error = "Cannot remove owned app command."; rollback(); return false;
    }
    removed = true;
    handoffTestPoint("menu-after-remove");
    if (!menuPathAbsent(activePath, error) || !menuPathAbsent(quickSwitchPath, error) ||
        !menuPathAbsent(pendingPath, error) || !menuPathAbsent(shutdownPath, error) ||
        (settings.automaticReturn && (!ticket.stillPublished(error) || !generation.stillPublished(error)))) {
        rollback(); return false;
    }
    // Runtime observes this last, after the launcher returns, just like Onion's
    // StartGameSwitcher shortcut. MENU does not write command/history data.
    const int fd = open(flagPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) { error = "Cannot publish GameSwitcher flag."; rollback(); return false; }
    const bool written = writeAll(fd, flagContents);
    flagOwned = fstat(fd, &ownedFlag) == 0;
    if (!flagOwned && written) {
        FileSnapshot identified;
        std::string inspectionError;
        if (readSnapshot(flagPath, identified, inspectionError) && identified.exists &&
            identified.contents == flagContents) {
            ownedFlag = identified.details; flagOwned = true;
        }
    }
    bool okay = written && flagOwned && fsync(fd) == 0;
    if (close(fd) != 0) okay = false;
    handoffTestPoint("menu-after-flag");
    FileSnapshot flag;
    okay = okay && readSnapshot(flagPath, flag, error) && flag.exists && flag.contents == flagContents &&
        flag.details.st_dev == ownedFlag.st_dev && flag.details.st_ino == ownedFlag.st_ino &&
        menuPathAbsent(activePath, error) && menuPathAbsent(pendingPath, error) &&
        menuPathAbsent(quickSwitchPath, error) && menuPathAbsent(shutdownPath, error);
    if (!okay) {
        if (error.empty()) error = "Cannot verify GameSwitcher flag publication.";
        rollback(); return false;
    }
    std::cerr << "Published Onion GameSwitcher request; history unchanged.\n";
    return true;
}

bool publishStagedOnionSwitcher(const std::string& requestDir, std::string& error) {
    return publishOnionSwitcherRequest(requestDir, kActiveCommand, kQuickSwitch,
        "/tmp/cmd_to_run.sh", "/tmp/.offOrder", kSdRoot, error);
}

bool requestOnionSwitcher(std::string& error) {
    error.clear();
    const char* requestDir = std::getenv("BETTER_FAVORITES_REQUEST_DIR");
    if (!requestDir || !validRequestDirectory(requestDir, error)) {
        if (error.empty()) error = "Private MENU request directory is unavailable.";
        return false;
    }
    std::string captured, active;
    if (!readCommand(appCommandPath(requestDir), captured, error) || !validAppCommand(captured) ||
        !readCommand(kActiveCommand, active, error) || active != captured) {
        if (error.empty()) error = "Active Onion command is not this app invocation.";
        return false;
    }
    AppSettings settings; std::string settingError;
    const char* env = std::getenv("BETTER_FAVORITES_SETTINGS");
    loadAppSettings(env && *env ? env : "/mnt/SDCARD/App/BetterFavoritesTest/settings.conf", settings, settingError);
    const char* capability = std::getenv("BETTER_FAVORITES_SWITCHER_HANDOFF");
    if (settings.automaticReturn && (!capability || std::string(capability) != "1")) {
        error = "Automatic return requires the MENU-capable runtime helper."; return false;
    }
    if (!regularFile("/mnt/SDCARD/.tmp_update/bin/gameSwitcher", error)) return false;
    return stageOnionSwitcherRequest(requestDir, error);
}
