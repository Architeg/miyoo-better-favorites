#ifndef BETTER_FAVORITES_LAUNCH_REQUEST_H
#define BETTER_FAVORITES_LAUNCH_REQUEST_H

#include "favorites.h"

#include <string>

// The outer launcher recognizes this only after main() has cleaned up SDL.
constexpr int kLaunchRequestedExitCode = 20;

bool buildOnionLaunchCommand(
    const Favorite& favorite,
    std::string& command,
    std::string& error
);

// Resolve and validate files without changing the command's stored paths.
// sdRoot can point to a mounted card or a filesystem fixture in host checks.
bool validateOnionLaunchFiles(
    const Favorite& favorite,
    const std::string& sdRoot,
    std::string& error
);

bool buildOnionRecentRecord(
    const Favorite& favorite,
    const std::string& sdRoot,
    std::string& record,
    std::string& error
);

bool stageOnionLaunchCommand(
    const std::string& command,
    const std::string& recentRecord,
    const std::string& requestDir,
    std::string& error
);

bool publishOnionLaunchCommand(
    const std::string& requestDir,
    const std::string& activePath,
    const std::string& quickSwitchPath,
    const std::string& sdRoot,
    std::string& error
);

#ifdef BETTER_FAVORITES_HANDOFF_TESTING
// Deterministic simulation of independent writers between publication steps.
void setOnionHandoffTestHook(void (*hook)(const char* phase));
#endif

bool cancelOnionLaunchCommand(
    const std::string& requestDir,
    std::string& error
);

bool publishStagedOnionLaunch(
    const std::string& requestDir,
    std::string& error
);

bool cancelStagedOnionLaunch(
    const std::string& requestDir,
    std::string& error
);

bool requestOnionLaunch(
    const Favorite& favorite,
    std::string& error
);

#endif
