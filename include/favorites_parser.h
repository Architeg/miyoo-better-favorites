#ifndef BETTER_FAVORITES_FAVORITES_PARSER_H
#define BETTER_FAVORITES_FAVORITES_PARSER_H

#include "favorites.h"
#include "settings.h"

#include <string>
#include <vector>

class FavoritesParser
{
public:
    explicit FavoritesParser(
        std::string sdRoot = "/mnt/SDCARD",
        AppSettings settings = {}
    );

    std::vector<Favorite> loadFavorites(
        const std::string& favoritesFile
    ) const;

    std::vector<SystemGroup> groupFavorites(
        const std::vector<Favorite>& favorites
    ) const;

    std::string displayLabel(
        const Favorite& favorite
    ) const;

private:
    std::string sdRoot_;
    AppSettings settings_;

    std::string extractSystemId(
        const std::string& launchPath
    ) const;

    std::string resolveSystemLabel(
        const std::string& systemId
    ) const;

    std::string mapSdPath(
        const std::string& path
    ) const;

    std::string makeSortKey(
        const std::string& label
    ) const;
};

#endif
