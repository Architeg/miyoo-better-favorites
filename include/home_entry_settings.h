#ifndef BETTER_FAVORITES_HOME_ENTRY_SETTINGS_H
#define BETTER_FAVORITES_HOME_ENTRY_SETTINGS_H
#include "settings.h"
bool loadHomeEntryPreference(const std::string&, AppSettings&, std::string&);
bool setHomeEntryPreference(const std::string&, bool, AppSettings&, std::string&);
// Full hashes are checked once per menu session and before a Home preference change, never per frame or normal startup.
HomeIntegrationStatus homeEntryStatus(const std::string& sdRoot, const std::string& appDirectory);
bool homeEntryAvailable(const std::string& sdRoot, const std::string& appDirectory);
std::string homeFileSha256(const std::string&);
// Session-scoped availability cache. Closing the menu invalidates the result;
// submenu Back reuses it. A preference write explicitly requests fresh verification.
class HomeStatusSession {
public:
    template<class Verify> HomeIntegrationStatus get(Verify verify, bool refresh=false) {
        if (!valid_ || refresh) { status_=verify(); valid_=true; }
        return status_;
    }
    void close() { valid_=false; }
private:
    bool valid_=false;
    HomeIntegrationStatus status_=HomeIntegrationStatus::NotInstalled;
};
#endif
