#include "launch_request.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <iostream>

#include <sys/stat.h>
#include <unistd.h>

namespace
{

const std::string appCommand =
    "cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; "
    "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh \n";

std::string readFile(const std::string& path)
{
    std::ifstream input(path, std::ios::binary);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    );
}

void writeFile(const std::string& path, const std::string& body)
{
    std::ofstream output(path, std::ios::binary);
    output << body;
    assert(output.good());
}

std::string gameSwitcherField(const std::string& record, const char* key)
{
    // The actual v4.3.1-1 parser: compact marker, then next raw quote.
    const std::string marker = "\"" + std::string(key) + "\":\"";
    const char* begin = std::strstr(record.c_str(), marker.c_str());
    assert(begin);
    begin += marker.size();
    const char* end = std::strchr(begin, '"');
    assert(end);
    return std::string(begin, end);
}

#ifdef BETTER_FAVORITES_HANDOFF_TESTING
std::string hookPhase;
std::string hookHistory;
std::string hookCommand;
std::string hookFlag;
std::string hookAction;
const std::string foreignRecord =
    "{\"label\":\"Other app\",\"launch\":\"other.sh\",\"type\":3}\n";

void externalWriter(const char* phase)
{
    if (hookPhase != phase) return;
    assert(access(hookFlag.c_str(), F_OK) != 0);
    if (hookAction == "history-before-publication") {
        writeFile(hookHistory + ".foreign", foreignRecord);
        assert(rename((hookHistory + ".foreign").c_str(),
                      hookHistory.c_str()) == 0);
        return;
    }
    if (hookAction == "append-history") {
        std::ofstream append(hookHistory, std::ios::app);
        append << foreignRecord;
        assert(append.good());
    } else if (hookAction == "prepend-history") {
        writeFile(hookHistory + ".foreign",
                  foreignRecord + readFile(hookHistory));
        assert(rename((hookHistory + ".foreign").c_str(),
                      hookHistory.c_str()) == 0);
    } else if (hookAction == "replace-both") {
        writeFile(hookHistory + ".foreign", foreignRecord);
        assert(rename((hookHistory + ".foreign").c_str(),
                      hookHistory.c_str()) == 0);
        writeFile(hookCommand, "unrelated command\n");
    } else if (hookAction == "replace-command") {
        writeFile(hookCommand, "unrelated command\n");
        return;
    }
    // Force flag publication failure without destroying this writer's flag.
    writeFile(hookFlag, "unrelated flag\n");
}
#endif

}

int main()
{
    Favorite favorite;
    favorite.launchPath = "/mnt/SDCARD/Emu/FC/launch.sh";
    favorite.romPath =
        "/mnt/SDCARD/Emu/FC/../../Roms/FC/Game (USA) [Rev A].zip";

    std::string command;
    std::string error;
    assert(buildOnionLaunchCommand(favorite, command, error));
    assert(command ==
        "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so "
        "\"/mnt/SDCARD/Emu/FC/launch.sh\" "
        "\"/mnt/SDCARD/Emu/FC/../../Roms/FC/Game (USA) [Rev A].zip\"\n");

    for (char unsafe : {'$', '`', '"', '\\', ':', ';', '&', '|', '\n'}) {
        Favorite invalid = favorite;
        invalid.romPath += unsafe;
        assert(!buildOnionLaunchCommand(invalid, command, error));
    }
    Favorite traversal = favorite;
    traversal.romPath = "/mnt/SDCARD/Roms/../Emu/FC/launch.sh";
    // Spelling alone does not determine whether a path escapes Roms.
    assert(buildOnionLaunchCommand(traversal, command, error));

    char cardTemplate[] = "/tmp/better-favorites-files.XXXXXX";
    char* cardDirectory = mkdtemp(cardTemplate);
    assert(cardDirectory);
    const std::string card = cardDirectory;
    for (const std::string& suffix : {
             "/Emu", "/Emu/GB", "/Roms", "/Roms/GB"}) {
        assert(mkdir((card + suffix).c_str(), 0700) == 0);
    }
    writeFile(card + "/Emu/GB/launch.sh", "#!/bin/sh\n");
    assert(chmod((card + "/Emu/GB/launch.sh").c_str(), 0700) == 0);
    writeFile(card + "/Roms/GB/Alleyway.gb", "fixture\n");

    // Exact rompath from the mounted favourite.json (line 23): the old
    // romSuffix.find("/./") check rejected this existing game.
    Favorite fromCard;
    fromCard.label = "Alleyway [US,JP,EU]";
    fromCard.launchPath = "/mnt/SDCARD/Emu/GB/launch.sh";
    fromCard.romPath =
        "/mnt/SDCARD/Emu/GB/../../Roms/GB/./Alleyway.gb";
    assert(buildOnionLaunchCommand(fromCard, command, error));
    assert(command ==
        "LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so "
        "\"/mnt/SDCARD/Emu/GB/launch.sh\" "
        "\"/mnt/SDCARD/Emu/GB/../../Roms/GB/./Alleyway.gb\"\n");
    assert(validateOnionLaunchFiles(fromCard, card, error));

    std::string record;
    assert(buildOnionRecentRecord(fromCard, card, record, error));
    assert(record.find("\"imgpath\":") == std::string::npos);
    assert(gameSwitcherField(record, "rompath") == fromCard.romPath);
    assert(gameSwitcherField(record, "launch") == fromCard.launchPath);
    int type = 0;
    assert(std::sscanf(std::strstr(record.c_str(), "\"type\":") + 7,
                       "%d", &type) == 1 && type == 5);

    Favorite withImage = fromCard;
    withImage.imagePath = "/mnt/SDCARD/Roms/GB/cover.png";
    assert(buildOnionRecentRecord(withImage, card, record, error));
    assert(record.find("\"imgpath\":") == std::string::npos);
    writeFile(card + "/Roms/GB/cover.png", "image fixture\n");
    assert(buildOnionRecentRecord(withImage, card, record, error));
    assert(gameSwitcherField(record, "imgpath") == withImage.imagePath);

    // Correct JSON escaping for metadata; GS uses the paths (and ROM cache
    // for its title), not the truncated/debug-only escaped-label extraction.
    withImage.label = "Pokémon \"quoted\" \\path\n\t";
    withImage.label += '\x01';
    assert(buildOnionRecentRecord(withImage, card, record, error));
    assert(gameSwitcherField(record, "rompath") == withImage.romPath);
    assert(gameSwitcherField(record, "launch") == withImage.launchPath);
    assert(gameSwitcherField(record, "imgpath") == withImage.imagePath);
    std::cout << record; // Independently parsed by Python in the host check.

    withImage.imagePath = "/mnt/SDCARD/Roms/GB/unsafe\\image.png";
    writeFile(card + "/Roms/GB/unsafe\\image.png", "image fixture\n");
    assert(!buildOnionRecentRecord(withImage, card, record, error));
    assert(unlink((card + "/Roms/GB/unsafe\\image.png").c_str()) == 0);
    assert(unlink((card + "/Roms/GB/cover.png").c_str()) == 0);
    Favorite tooLong = fromCard;
    tooLong.romPath += std::string(256, 'x');
    assert(!buildOnionRecentRecord(tooLong, card, record, error));
    tooLong = fromCard;
    tooLong.label = std::string(256, 'x');
    assert(!buildOnionRecentRecord(tooLong, card, record, error));
    assert(buildOnionRecentRecord(fromCard, card, record, error));

    Favorite withinRoms = fromCard;
    withinRoms.romPath = "/mnt/SDCARD/Roms/GB/../GB/./Alleyway.gb";
    assert(buildOnionLaunchCommand(withinRoms, command, error));
    assert(validateOnionLaunchFiles(withinRoms, card, error));

    Favorite escaped = fromCard;
    escaped.romPath = "/mnt/SDCARD/Roms/../Emu/GB/launch.sh";
    assert(buildOnionLaunchCommand(escaped, command, error));
    assert(!validateOnionLaunchFiles(escaped, card, error));
    assert(error ==
        "Resolved launch or ROM path is outside Onion's directories.");

    assert(symlink("../../Emu/GB/launch.sh",
                   (card + "/Roms/GB/escape.gb").c_str()) == 0);
    escaped.romPath = "/mnt/SDCARD/Roms/GB/escape.gb";
    assert(!validateOnionLaunchFiles(escaped, card, error));

    writeFile(card + "/Roms/GB/unsafe$target.gb", "fixture\n");
    assert(symlink("unsafe$target.gb",
                   (card + "/Roms/GB/alias.gb").c_str()) == 0);
    escaped.romPath = "/mnt/SDCARD/Roms/GB/alias.gb";
    assert(buildOnionLaunchCommand(escaped, command, error));
    assert(!validateOnionLaunchFiles(escaped, card, error));
    assert(error ==
        "Resolved launch or ROM path contains unsupported characters.");

    Favorite missing = fromCard;
    missing.romPath = "/mnt/SDCARD/Roms/GB/missing.gb";
    assert(!validateOnionLaunchFiles(missing, card, error));
    missing.romPath = "/mnt/SDCARD/Roms/GB/";
    assert(!validateOnionLaunchFiles(missing, card, error));
    assert(chmod((card + "/Emu/GB/launch.sh").c_str(), 0600) == 0);
    assert(!validateOnionLaunchFiles(fromCard, card, error));
    assert(error == "Onion launch script is not executable.");

    assert(unlink((card + "/Roms/GB/escape.gb").c_str()) == 0);
    assert(unlink((card + "/Roms/GB/alias.gb").c_str()) == 0);
    assert(unlink((card + "/Roms/GB/unsafe$target.gb").c_str()) == 0);
    assert(unlink((card + "/Roms/GB/Alleyway.gb").c_str()) == 0);
    assert(unlink((card + "/Emu/GB/launch.sh").c_str()) == 0);
    for (const std::string& suffix : {
             "/Emu/GB", "/Emu", "/Roms/GB", "/Roms"}) {
        assert(rmdir((card + suffix).c_str()) == 0);
    }
    assert(rmdir(card.c_str()) == 0);

    char requestTemplate[] = "/tmp/better-favorites.XXXXXX";
    char* requestDir = mkdtemp(requestTemplate);
    assert(requestDir);
    char destinationTemplate[] = "/tmp/better-favorites-handoff-test.XXXXXX";
    char* destinationDir = mkdtemp(destinationTemplate);
    assert(destinationDir);

    const std::string stage = std::string(requestDir) + "/request.sh";
    const std::string captured = std::string(requestDir) + "/app-command.sh";
    const std::string active = std::string(destinationDir) + "/cmd_to_run.sh";
    const std::string flag = std::string(destinationDir) + "/quick_switch";
    std::string game;
    assert(buildOnionLaunchCommand(fromCard, game, error));
    for (const std::string& suffix : {
             "/.tmp_update", "/.tmp_update/config", "/Roms"}) {
        assert(mkdir((std::string(destinationDir) + suffix).c_str(), 0700) == 0);
    }
    const std::string visible = std::string(destinationDir) + "/Roms/recentlist.json";
    const std::string hidden = std::string(destinationDir) + "/Roms/recentlist-hidden.json";
    const std::string showRecents = std::string(destinationDir) +
        "/.tmp_update/config/.showRecents";
    const std::string oldHistory =
        "{\"label\":\"Existing\",\"launch\":\"existing.sh\",\"type\":3}\n";
    writeFile(visible, oldHistory);
    writeFile(hidden, oldHistory);
    writeFile(captured, appCommand);
    writeFile(active, appCommand);

    // Abnormal exit or INT/TERM removes only this invocation's private files.
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(access(stage.c_str(), F_OK) != 0);
    assert(access((std::string(requestDir) + "/recent.json").c_str(), F_OK) != 0);
    assert(readFile(visible) == oldHistory);
    assert(readFile(hidden) == oldHistory);
    assert(readFile(active) == appCommand);
    writeFile(captured, appCommand);

    // The normal handoff replaces the app command and then sets the flag.
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(publishOnionLaunchCommand(
        requestDir, active, flag, destinationDir, error));
    assert(readFile(active) == game);
    assert(readFile(hidden) == record + oldHistory);
    assert(readFile(visible) == oldHistory);
    assert(access(flag.c_str(), F_OK) == 0);
    struct stat details {};
    assert(stat(active.c_str(), &details) == 0);
    assert((details.st_mode & S_IXUSR) != 0);
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(readFile(active) == game);
    assert(unlink(flag.c_str()) == 0);

    writeFile(hidden, oldHistory);

    // Visible Recents uses the other destination; preserve hidden history.
    writeFile(showRecents, "");
    writeFile(captured, appCommand);
    writeFile(active, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(publishOnionLaunchCommand(
        requestDir, active, flag, destinationDir, error));
    assert(readFile(visible) == record + oldHistory);
    assert(readFile(hidden) == oldHistory);
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(unlink(flag.c_str()) == 0);
    assert(unlink(showRecents.c_str()) == 0);
    writeFile(visible, oldHistory);

    // Missing history is created only by a successful handoff.
    assert(unlink(hidden.c_str()) == 0);
    writeFile(captured, appCommand);
    writeFile(active, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(publishOnionLaunchCommand(
        requestDir, active, flag, destinationDir, error));
    assert(readFile(hidden) == record);
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(unlink(flag.c_str()) == 0);
    writeFile(hidden, oldHistory);

    // Never replace or remove an unrelated active Onion command.
    writeFile(captured, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    writeFile(active, "unrelated\n");
    assert(!publishOnionLaunchCommand(requestDir, active, flag, destinationDir, error));
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(readFile(active) == "unrelated\n");
    assert(readFile(hidden) == oldHistory);
    assert(readFile(visible) == oldHistory);
    assert(access(flag.c_str(), F_OK) != 0);

    // A preexisting quick-switch flag must survive a failed publication.
    writeFile(captured, appCommand);
    writeFile(active, appCommand);
    writeFile(flag, "unrelated flag\n");
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(!publishOnionLaunchCommand(requestDir, active, flag, destinationDir, error));
    assert(readFile(active) == appCommand);
    assert(readFile(flag) == "unrelated flag\n");
    assert(readFile(hidden) == oldHistory);
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(unlink(flag.c_str()) == 0);

    // Flag creation failure after replacement restores the app command.
    writeFile(captured, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    const std::string impossibleFlag =
        std::string(destinationDir) + "/missing/quick_switch";
    assert(!publishOnionLaunchCommand(
        requestDir, active, impossibleFlag, destinationDir, error));
    assert(readFile(active) == appCommand);
    assert(readFile(hidden) == oldHistory);
    assert(readFile(visible) == oldHistory);
    assert(cancelOnionLaunchCommand(requestDir, error));

    // A newly created history is removed when the subsequent flag fails.
    assert(unlink(hidden.c_str()) == 0);
    writeFile(captured, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(!publishOnionLaunchCommand(
        requestDir, active, impossibleFlag, destinationDir, error));
    assert(access(hidden.c_str(), F_OK) != 0);
    assert(readFile(active) == appCommand);
    assert(cancelOnionLaunchCommand(requestDir, error));
    writeFile(hidden, oldHistory);

#ifdef BETTER_FAVORITES_HANDOFF_TESTING
    hookHistory = hidden;
    hookCommand = active;
    hookFlag = flag;
    setOnionHandoffTestHook(externalWriter);

    // Another writer changes history before we publish it: do not overwrite.
    hookPhase = "after-command";
    hookAction = "history-before-publication";
    writeFile(captured, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(!publishOnionLaunchCommand(
        requestDir, active, flag, destinationDir, error));
    assert(readFile(hidden) == foreignRecord);
    assert(readFile(active) == appCommand);
    assert(access(flag.c_str(), F_OK) != 0);
    assert(cancelOnionLaunchCommand(requestDir, error));

    // Remove only our insertion, retaining unrelated appends and atomic
    // prepends that happen after history publication but before flag creation.
    hookPhase = "after-history";
    for (const std::string& action : {"append-history", "prepend-history"}) {
        hookAction = action;
        writeFile(hidden, oldHistory);
        writeFile(active, appCommand);
        writeFile(captured, appCommand);
        assert(stageOnionLaunchCommand(game, record, requestDir, error));
        assert(!publishOnionLaunchCommand(
            requestDir, active, flag, destinationDir, error));
        assert(readFile(hidden) == (action == "append-history"
            ? oldHistory + foreignRecord : foreignRecord + oldHistory));
        assert(readFile(active) == appCommand);
        assert(readFile(flag) == "unrelated flag\n");
        assert(cancelOnionLaunchCommand(requestDir, error));
        assert(unlink(flag.c_str()) == 0);
    }

    // A command replaced after registration must not receive our flag.
    hookAction = "replace-command";
    writeFile(hidden, oldHistory);
    writeFile(active, appCommand);
    writeFile(captured, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(!publishOnionLaunchCommand(
        requestDir, active, flag, destinationDir, error));
    assert(readFile(hidden) == oldHistory);
    assert(readFile(active) == "unrelated command\n");
    assert(access(flag.c_str(), F_OK) != 0);
    assert(cancelOnionLaunchCommand(requestDir, error));

    // A complete unrelated replacement of both files must also survive.
    hookAction = "replace-both";
    writeFile(hidden, oldHistory);
    writeFile(active, appCommand);
    writeFile(captured, appCommand);
    assert(stageOnionLaunchCommand(game, record, requestDir, error));
    assert(!publishOnionLaunchCommand(
        requestDir, active, flag, destinationDir, error));
    assert(readFile(hidden) == foreignRecord);
    assert(readFile(active) == "unrelated command\n");
    assert(readFile(flag) == "unrelated flag\n");
    assert(cancelOnionLaunchCommand(requestDir, error));
    assert(unlink(flag.c_str()) == 0);
    setOnionHandoffTestHook(nullptr);
#endif

    assert(unlink(visible.c_str()) == 0);
    assert(unlink(hidden.c_str()) == 0);
    for (const std::string& suffix : {
             "/.tmp_update/config", "/.tmp_update", "/Roms"}) {
        assert(rmdir((std::string(destinationDir) + suffix).c_str()) == 0);
    }
    assert(unlink(active.c_str()) == 0);
    assert(rmdir(destinationDir) == 0);
    assert(rmdir(requestDir) == 0);
}
