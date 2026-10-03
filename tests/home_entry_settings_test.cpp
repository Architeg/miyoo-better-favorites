#include "home_entry_settings.h"
#include <cassert>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
void write(const std::string& p,const std::string& s){std::ofstream(p)<<s;}
int main(){
 char directory[]="/tmp/bf-home-settings.XXXXXX";assert(mkdtemp(directory));std::string root=directory,path=root+"/home-entry.conf",error;AppSettings s;
 s.automaticReturn=true;s.returnGeneration="unchanged";s.groupByConsole=false;
 assert(loadHomeEntryPreference(path,s,error)&&!s.replaceStockFavorites);
 assert(setHomeEntryPreference(path,true,s,error));assert(s.replaceStockFavorites);
 assert(loadHomeEntryPreference(path,s,error)&&s.replaceStockFavorites);
 assert(s.automaticReturn&&s.returnGeneration=="unchanged"&&!s.groupByConsole);
 assert(homeEntryStatus(root,root)==HomeIntegrationStatus::Unavailable);
 assert(!homeEntryAvailable(root,root)); // ON preference is not installation
 write(path,"BetterFavoritesHome1\n1\nextra");assert(!loadHomeEntryPreference(path,s,error)&&!s.replaceStockFavorites);
 assert(setHomeEntryPreference(path,true,s,error));assert(!setHomeEntryPreference(root+"/missing/file",false,s,error)&&s.replaceStockFavorites);
 unlink(path.c_str());assert(mkfifo(path.c_str(),0600)==0);alarm(2);assert(!loadHomeEntryPreference(path,s,error));alarm(0);s.replaceStockFavorites=true;
 assert(!setHomeEntryPreference(path,false,s,error)&&s.replaceStockFavorites);unlink(path.c_str());
 write(path,"abc");assert(homeFileSha256(path)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
 write(path,"");assert(homeFileSha256(path)=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
 write(path,std::string(1000000,'a'));assert(homeFileSha256(path)=="cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
 unlink(path.c_str());symlink("unavailable",path.c_str());assert(!setHomeEntryPreference(path,false,s,error));assert(homeFileSha256(path).empty());unlink(path.c_str());rmdir(root.c_str());
 std::cout<<"Home preference independent/default-off, malformed/FIFO/write failures, installation separation and SHA256: PASS\n";
}
