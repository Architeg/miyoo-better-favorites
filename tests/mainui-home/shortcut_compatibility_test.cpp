// Investigation fixture only. Does not relax production command ownership.
#include "launch_request.h"
#include <cassert>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unistd.h>
static void write(const std::string& p,const std::string& s){std::ofstream f(p);f<<s;assert(f.good());}
static std::string read(const std::string& p){std::ifstream f(p);return {std::istreambuf_iterator<char>(f),{}};}
int main(){
 char temp[]="/tmp/bf-shortcut-compat.XXXXXX",request[]="/tmp/better-favorites.XXXXXX";
 assert(mkdtemp(temp)&&mkdtemp(request));const std::string root=temp,req=request;
 assert(mkdir((root+"/.tmp_update").c_str(),0700)==0);assert(mkdir((root+"/Roms").c_str(),0700)==0);
 const std::string active=root+"/.tmp_update/cmd_to_run.sh",quick=root+"/quick",pending=root+"/pending",shutdown=root+"/shutdown";
 // Exact set_cmd_app output in tagged Onion v4.3.1-1 apps.h for this app.
 const std::string command="cd /mnt/SDCARD/App/BetterFavorites; chmod a+x ./launch.sh; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so ./launch.sh";
 std::string error;write(active,command);write(req+"/app-command.sh",command);
 assert(stageOnionLaunchCommand("game fixture\n","{\"type\":5}\n",req,error));
 assert(!publishOnionLaunchCommand(req,active,quick,root,error));
 assert(read(active)==command&&access(quick.c_str(),F_OK)!=0);assert(cancelOnionLaunchCommand(req,error));
 write(req+"/app-command.sh",command);assert(stageOnionSwitcherRequest(req,error));
 assert(!publishOnionSwitcherRequest(req,active,quick,pending,shutdown,root,error));
 assert(read(active)==command&&access(quick.c_str(),F_OK)!=0&&access((root+"/.tmp_update/.runGameSwitcher").c_str(),F_OK)!=0);
 assert(cancelOnionLaunchCommand(req,error));assert(unlink(active.c_str())==0);assert(rmdir((root+"/.tmp_update").c_str())==0);assert(rmdir((root+"/Roms").c_str())==0);assert(rmdir(root.c_str())==0);assert(rmdir(req.c_str())==0);
 std::cout<<"Stock keymon one-space command rejected by unchanged A/MENU ownership guards: PASS\n";
}
