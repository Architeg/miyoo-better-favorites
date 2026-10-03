#ifndef BETTER_FAVORITES_HOME_ENTRY_SETTINGS_H
#define BETTER_FAVORITES_HOME_ENTRY_SETTINGS_H
#include "settings.h"
bool loadHomeEntryPreference(const std::string&, AppSettings&, std::string&);
bool setHomeEntryPreference(const std::string&, bool, AppSettings&, std::string&);
// Full hashes are checked once per Settings entry, never per frame or normal startup.
HomeIntegrationStatus homeEntryStatus(const std::string& sdRoot, const std::string& appDirectory);
bool homeEntryAvailable(const std::string& sdRoot, const std::string& appDirectory);
std::string homeFileSha256(const std::string&);
#endif
