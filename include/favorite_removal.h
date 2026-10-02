#ifndef BETTER_FAVORITES_FAVORITE_REMOVAL_H
#define BETTER_FAVORITES_FAVORITE_REMOVAL_H
#include "favorites.h"
#include <sys/stat.h>
#include <string>
struct FavoritesSnapshot {
    std::string bytes;
    struct stat identity {};
    bool valid = false;
};
bool readFavoritesSnapshot(const std::string& path, FavoritesSnapshot& snapshot, std::string& error);
// Removes exactly the parser's source record. Preserves every other byte.
// A successful publication always has a verified, durable backup beside the source.
bool removeFavorite(const std::string& path, FavoritesSnapshot& snapshot,
                    const Favorite& favorite, std::string& backup, std::string& error);
#ifdef BETTER_FAVORITES_REMOVAL_TESTING
extern void (*favoriteRemovalTestHook)(const char* phase);
#endif
#endif
