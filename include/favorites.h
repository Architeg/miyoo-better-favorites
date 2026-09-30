#ifndef BETTER_FAVORITES_FAVORITES_H
#define BETTER_FAVORITES_FAVORITES_H

#include <string>
#include <vector>

struct Favorite
{
    // Values stored directly in Onion's favourite.json.
    std::string label;
    std::string launchPath;
    std::string romPath;
    std::string imagePath;

    // Resolved dynamically from the launch path / emulator config.
    std::string systemId;
    std::string systemLabel;
};

struct SystemGroup
{
    std::string id;
    std::string label;

    std::vector<Favorite> favorites;
};

#endif
