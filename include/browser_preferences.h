#ifndef BETTER_FAVORITES_BROWSER_PREFERENCES_H
#define BETTER_FAVORITES_BROWSER_PREFERENCES_H
#include "settings.h"
bool loadBrowserPreferences(const std::string& path, AppSettings& settings, std::string& error);
bool saveBrowserPreferences(const std::string& path, const AppSettings& candidate,
                            AppSettings& saved, std::string& error);
#ifdef BETTER_FAVORITES_PREFERENCES_TESTING
extern void (*browserPreferencesTestHook)(const char* phase);
#endif
#endif
