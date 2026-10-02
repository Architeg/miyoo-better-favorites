#include "browser_preferences.h"
#include "settings.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
std::string root;
std::string read(const std::string& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void write(const std::string& p,const std::string& b){std::ofstream f(p,std::ios::binary);f<<b;assert(f.good());}
void hook(const char*){assert(chmod(root.c_str(),0500)==0);}
int main(){
 char dir[]="/tmp/better-favorites-preferences-test.XXXXXX";assert(mkdtemp(dir));root=dir;
 const auto path=root+"/browser-preferences.conf",ret=root+"/settings.conf";
 const auto favorite=root+"/favourite.json",history=root+"/recentlist.json";
 write(favorite,"untouched favorites");write(history,"untouched history");
 AppSettings saved;std::string error;
 assert(setAutomaticReturn(ret,true,saved,error));const auto returnBytes=read(ret),generation=saved.returnGeneration;
 // Migration means a legacy return file alone uses browser defaults unchanged.
 AppSettings loaded;assert(loadAppSettings(ret,loaded,error));assert(loadBrowserPreferences(path,loaded,error));
 assert(loaded.groupByConsole&&loaded.showNumericPrefixes&&loaded.sortMode==SortMode::OriginalLabel&&loaded.automaticReturn&&loaded.returnGeneration==generation);
 for(int bits=0;bits<8;++bits){AppSettings next=saved;next.groupByConsole=bits&1;next.showNumericPrefixes=bits&2;next.sortMode=bits&4?SortMode::AlphabeticalTitle:SortMode::OriginalLabel;
  assert(saveBrowserPreferences(path,next,saved,error));assert(loadBrowserPreferences(path,loaded,error));
  assert(loaded.groupByConsole==next.groupByConsole&&loaded.showNumericPrefixes==next.showNumericPrefixes&&loaded.sortMode==next.sortMode);
  assert(saved.automaticReturn&&saved.returnGeneration==generation&&read(ret)==returnBytes);
 }
 for(const auto& bad:{"","BetterFavoritesBrowserPreferences2\n0\n0\n1\n","BetterFavoritesBrowserPreferences1\n2\n0\n1\n","BetterFavoritesBrowserPreferences1\n0\n0\n1", "BetterFavoritesBrowserPreferences1\n0\n0\n2\n", "BetterFavoritesBrowserPreferences1\n0\n0\n1\nextra"}){
  write(path,bad);assert(!loadBrowserPreferences(path,loaded,error));assert(loaded.groupByConsole&&loaded.showNumericPrefixes&&loaded.sortMode==SortMode::OriginalLabel);assert(loaded.automaticReturn&&loaded.returnGeneration==generation);
 }
 write(path,std::string(256,'x'));assert(!loadBrowserPreferences(path,loaded,error));
 AppSettings candidate=saved;candidate.groupByConsole=!saved.groupByConsole;
 assert(saveBrowserPreferences(path,saved,loaded,error));const auto original=read(path);const auto oldGroup=saved.groupByConsole;
 assert(!saveBrowserPreferences(root+"/missing/prefs",candidate,saved,error));assert(saved.groupByConsole==oldGroup);
 auto invalid=candidate;invalid.sortMode=static_cast<SortMode>(5);assert(!saveBrowserPreferences(path,invalid,saved,error));assert(read(path)==original&&saved.groupByConsole==oldGroup);
 if(geteuid()!=0){browserPreferencesTestHook=hook;assert(!saveBrowserPreferences(path,candidate,saved,error));assert(chmod(root.c_str(),0700)==0);assert(read(path)==original&&saved.groupByConsole==oldGroup);browserPreferencesTestHook=nullptr;}
 assert(rename(path.c_str(),(path+".target").c_str())==0);assert(symlink((path+".target").c_str(),path.c_str())==0);
 assert(!loadBrowserPreferences(path,loaded,error));assert(!saveBrowserPreferences(path,candidate,saved,error));assert(read(path+".target")==original);
 assert(read(favorite)=="untouched favorites"&&read(history)=="untouched history"&&read(ret)==returnBytes);
 // A directory permission failure can also prevent removing its own temporary.
 // Restore access above, then clean only this isolated fixture directory.
 auto* entries=opendir(root.c_str());assert(entries);
 while(auto* entry=readdir(entries)){const std::string name=entry->d_name;if(name.find("browser-preferences.conf.")==0)assert(unlink((root+"/"+name).c_str())==0);}
 closedir(entries);
 unlink(favorite.c_str());unlink(history.c_str());
 unlink(path.c_str());unlink((path+".target").c_str());unlink(ret.c_str());assert(rmdir(root.c_str())==0);
 std::cout<<"Browser preference defaults/migration, all combinations, malformed/oversize, atomic write failure and unchanged return protocol: PASS\n";
}
