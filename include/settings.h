#ifndef BETTER_FAVORITES_SETTINGS_H
#define BETTER_FAVORITES_SETTINGS_H

#include <string>

enum class SortMode
{
    OriginalLabel,
    AlphabeticalTitle
};

struct AppSettings
{
    bool replaceStockFavorites = false;
    bool homeIntegrationAvailable = false; // inspected, never persisted as preference
    bool automaticReturn = false;
    std::string returnGeneration;

    bool showNumericPrefixes = true;
    bool groupByConsole = true;

    SortMode sortMode = SortMode::OriginalLabel;
};

bool loadAppSettings(const std::string& path, AppSettings& settings, std::string& error);
bool automaticReturnAvailable();
bool setAutomaticReturn(const std::string& path, bool enabled, AppSettings& settings,
                        std::string& error);

#endif
