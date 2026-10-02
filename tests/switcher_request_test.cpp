#include "launch_request.h"
#include "settings.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <string>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
static std::string active, flag, pending, shutdown, fault;
static void write(const std::string& path,const std::string& body) { std::ofstream out(path);out<<body;assert(out.good()); }
static std::string read(const std::string& path) { std::ifstream in(path);return {std::istreambuf_iterator<char>(in),{}}; }
static void hook(const char* phase) {
    if (fault=="replace-active" && std::string(phase)=="menu-before-remove") write(active,"foreign command");
    if (fault=="active-after-remove" && std::string(phase)=="menu-after-remove") write(active,"foreign command");
    if (fault=="flag-after-remove" && std::string(phase)=="menu-after-remove") write(flag,"foreign flag");
    if (fault=="shutdown" && std::string(phase)=="menu-after-flag") write(shutdown,"shutdown");
    if (fault=="partial-flag" && std::string(phase)=="menu-after-flag") write(flag,read(flag).substr(0,7));
    if (fault=="foreign-flag" && std::string(phase)=="menu-after-flag") write(flag,"foreign flag");
}
int main() {
    char request[]="/tmp/better-favorites.XXXXXX",card[]="/tmp/better-favorites-menu-test.XXXXXX";
    assert(mkdtemp(request)&&mkdtemp(card));
    const std::string root=card, dir=request;
    assert(mkdir((root+"/.tmp_update").c_str(),0700)==0);
    assert(mkdir((root+"/.tmp_update/bin").c_str(),0700)==0);
    assert(mkdir((root+"/Roms").c_str(),0700)==0);
    write(root+"/.tmp_update/bin/gameSwitcher","#!/bin/sh\nexit 0\n");
    assert(chmod((root+"/.tmp_update/bin/gameSwitcher").c_str(),0700)==0);
    const std::string command="cd /mnt/SDCARD/App/BetterFavoritesTest; chmod a+x ./launch.sh; LD_PRELOAD=/mnt/SDCARD/miyoo/app/../lib/libpadsp.so   ./launch.sh\n";
    active=root+"/.tmp_update/cmd_to_run.sh";flag=root+"/.tmp_update/.runGameSwitcher";
    pending=root+"/pending";shutdown=root+"/shutdown";
    const std::string quick=root+"/quick_switch",settings=root+"/settings.conf";
    assert(setenv("BETTER_FAVORITES_SETTINGS",settings.c_str(),1)==0);
    write(root+"/Roms/recentlist.json","");write(root+"/Roms/recentlist-hidden.json","existing record\n");
    std::string error;
    auto stage=[&](){write(dir+"/app-command.sh",command);write(active,command);assert(stageOnionSwitcherRequest(dir,error));};
    auto publish=[&](){return publishOnionSwitcherRequest(dir,active,quick,pending,shutdown,root,error);};
    auto cleanup=[&](){assert(cancelOnionLaunchCommand(dir,error));};
    stage();assert(!stageOnionSwitcherRequest(dir,error));cleanup();
    assert(read(active)==command&&access(flag.c_str(),F_OK)!=0); // normal B/cancellation
    stage();assert(publish());assert(access(active.c_str(),F_OK)!=0&&read(flag)=="BetterFavoritesSwitcher1\n"+dir+"\n");cleanup();unlink(flag.c_str());
    assert(read(root+"/Roms/recentlist.json").empty()&&read(root+"/Roms/recentlist-hidden.json")=="existing record\n");
    // Refuse an unrelated pending request, occupied flag or shutdown before removal.
    for(const auto& blocker:{pending,quick,flag,shutdown}) {stage();write(blocker,"foreign");assert(!publish());assert(read(active)==command&&read(blocker)=="foreign");unlink(blocker.c_str());cleanup();}
    char context[]="/tmp/better-favorites-return.XXXXXX";assert(mkdtemp(context));
    assert(setenv("BETTER_FAVORITES_RETURN_DIR",context,1)==0);
    AppSettings app;assert(setAutomaticReturn(settings,true,app,error));
    stage();assert(!publish());assert(read(active)==command);cleanup(); // old runtime capability
    assert(setenv("BETTER_FAVORITES_SWITCHER_HANDOFF","1",1)==0);
    const std::string ticket=std::string(context)+"/switcher.request", generation=std::string(context)+"/generation";
    for(const char* failure:{"replace-active","active-after-remove","flag-after-remove","shutdown","foreign-flag","partial-flag"}) {
        fault=failure;stage();setOnionHandoffTestHook(hook);assert(!publish());setOnionHandoffTestHook(nullptr);
        assert(access(ticket.c_str(),F_OK)!=0&&access(generation.c_str(),F_OK)!=0);
        if(fault=="replace-active"||fault=="active-after-remove") assert(read(active)=="foreign command");
        else assert(read(active)==command);
        if(fault=="flag-after-remove"||fault=="foreign-flag") assert(read(flag)=="foreign flag");
        else assert(access(flag.c_str(),F_OK)!=0);
        unlink(flag.c_str());unlink(shutdown.c_str());cleanup();
    }
    fault.clear();stage();assert(publish());assert(read(ticket)==read(flag));assert(read(generation)==app.returnGeneration+"\n");cleanup();
    assert(read(root+"/Roms/recentlist.json").empty()&&read(root+"/Roms/recentlist-hidden.json")=="existing record\n");
    unlink(ticket.c_str());unlink(generation.c_str());rmdir(context);unlink(flag.c_str());
    unsetenv("BETTER_FAVORITES_SETTINGS");unsetenv("BETTER_FAVORITES_RETURN_DIR");unsetenv("BETTER_FAVORITES_SWITCHER_HANDOFF");
    for(const auto& file:{root+"/Roms/recentlist.json",root+"/Roms/recentlist-hidden.json",root+"/.tmp_update/bin/gameSwitcher",settings})unlink(file.c_str());
    rmdir((root+"/Roms").c_str());rmdir((root+"/.tmp_update/bin").c_str());rmdir((root+"/.tmp_update").c_str());rmdir(card);rmdir(request);
}
