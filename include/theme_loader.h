#ifndef BETTER_FAVORITES_THEME_LOADER_H
#define BETTER_FAVORITES_THEME_LOADER_H

#include "theme.h"

#include <string>

class ThemeLoader
{
public:
    explicit ThemeLoader(
        std::string sdRoot = "/mnt/SDCARD"
    );

    Theme load() const;

private:
    std::string sdRoot_;

    std::string readActiveThemePath() const;

    std::string mapSdPath(
        const std::string& path
    ) const;

    std::string resolveThemePath(
        const std::string& themeRoot,
        const std::string& path
    ) const;
};

#endif
