#ifndef BETTER_FAVORITES_THEME_H
#define BETTER_FAVORITES_THEME_H

#include <string>

struct ThemeTextStyle
{
    std::string fontPath;
    int size = 20;

    int red = 255;
    int green = 255;
    int blue = 255;
};

struct Theme
{
    // Full active theme directory, for example:
    // /mnt/SDCARD/Themes/Some Theme/
    std::string rootPath;

    bool hideIcons = false;
    bool hideHints = false;

    // Common Onion skin assets.
    std::string backgroundPath;
    std::string titleBackgroundPath;
    std::string footerBackgroundPath;

    std::string selectedItemPath;
    std::string normalItemPath;

    std::string listSmallPath;
    std::string listLargePath;

    std::string horizontalDividerPath;

    std::string buttonAPath;
    std::string buttonBPath;

    // Text styles from config.json.
    ThemeTextStyle title;
    ThemeTextStyle list;
    ThemeTextStyle hint;

    /*
     * Console section headers.
     * Defaults are derived from the active list style later,
     * so themes do not need a new config key.
     */
    ThemeTextStyle section;

    int selectedRed = 208;
    int selectedGreen = 208;
    int selectedBlue = 208;

    int currentPageRed = 132;
    int currentPageGreen = 132;
    int currentPageBlue = 132;

    int totalRed = 132;
    int totalGreen = 132;
    int totalBlue = 132;
};

#endif
