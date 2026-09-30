#ifndef BETTER_FAVORITES_SETTINGS_H
#define BETTER_FAVORITES_SETTINGS_H

enum class SortMode
{
    OriginalLabel,
    AlphabeticalTitle
};

struct AppSettings
{
    bool showNumericPrefixes = true;
    bool groupByConsole = true;

    SortMode sortMode = SortMode::OriginalLabel;
};

#endif
