#include "browser_preferences.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
namespace {
std::string encode(const AppSettings& s){return std::string("BetterFavoritesBrowserPreferences1\n")+(s.groupByConsole?"1\n":"0\n")+(s.showNumericPrefixes?"1\n":"0\n")+(s.sortMode==SortMode::OriginalLabel?"0\n":"1\n");}
void copy(const AppSettings& a,AppSettings& b){b.groupByConsole=a.groupByConsole;b.showNumericPrefixes=a.showNumericPrefixes;b.sortMode=a.sortMode;}
}
#ifdef BETTER_FAVORITES_PREFERENCES_TESTING
void (*browserPreferencesTestHook)(const char*)=nullptr;
#endif
bool loadBrowserPreferences(const std::string& path,AppSettings& s,std::string& error){
    copy(AppSettings{},s);error.clear();
    int fd=open(path.c_str(),O_RDONLY|O_NOFOLLOW|O_NONBLOCK);
    if(fd<0){if(errno==ENOENT)return true;error="Cannot read browser preferences; using defaults.";return false;}
    struct stat info{};std::string body;char buffer[256];
    bool okay=fstat(fd,&info)==0 && S_ISREG(info.st_mode) && info.st_size<=128;
    while(okay){const auto n=read(fd,buffer,sizeof(buffer));if(n<0&&errno==EINTR)continue;if(n<0){okay=false;break;}if(!n)break;body.append(buffer,n);if(body.size()>128)okay=false;}
    if(close(fd)!=0)okay=false;
    for(int bits=0;okay&&bits<8;++bits){AppSettings candidate;candidate.groupByConsole=bits&1;candidate.showNumericPrefixes=bits&2;candidate.sortMode=bits&4?SortMode::AlphabeticalTitle:SortMode::OriginalLabel;if(body==encode(candidate)){copy(candidate,s);return true;}}
    error="Malformed browser preferences; using defaults.";return false;
}
bool saveBrowserPreferences(const std::string& path,const AppSettings& candidate,AppSettings& saved,std::string& error){
    error.clear();
    if(candidate.sortMode!=SortMode::OriginalLabel&&candidate.sortMode!=SortMode::AlphabeticalTitle){error="Invalid browser sorting preference.";return false;}
    struct stat old{};
    if(lstat(path.c_str(),&old)==0){if(!S_ISREG(old.st_mode)){error="Browser preferences path is not a regular file.";return false;}}
    else if(errno!=ENOENT){error="Cannot inspect browser preferences.";return false;}
    std::string temp=path+".XXXXXX";int fd=mkstemp(&temp[0]);
    if(fd<0){error="Cannot stage browser preferences.";return false;}
    const auto body=encode(candidate);bool okay=fchmod(fd,0600)==0;std::size_t offset=0;
    while(okay&&offset<body.size()){const auto n=write(fd,body.data()+offset,body.size()-offset);if(n<0&&errno==EINTR)continue;if(n<=0){okay=false;break;}offset+=n;}
    if(okay)okay=fsync(fd)==0;
    if(close(fd)!=0)okay=false;
    AppSettings verified;std::string detail;
    okay=okay&&loadBrowserPreferences(temp,verified,detail)&&encode(verified)==body;
#ifdef BETTER_FAVORITES_PREFERENCES_TESTING
    if(browserPreferencesTestHook)browserPreferencesTestHook("before-publication");
#endif
    if(okay)okay=rename(temp.c_str(),path.c_str())==0;
    if(!okay){
        error="Cannot save browser preferences. Previous setting kept.";
        if(unlink(temp.c_str())!=0 && errno!=ENOENT)error+=" Staged file cleanup failed.";
        return false;
    }
    copy(candidate,saved);return true;
}
