#include "launch_request.h"
#include "settings.h"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sys/stat.h>
#include <unistd.h>
namespace fs=std::filesystem;
static void write(const std::string& p,const std::string& s) { std::ofstream f(p); f<<s; assert(f.good()); }
static std::string read(const std::string& p) { std::ifstream f(p); return {std::istreambuf_iterator<char>(f),{}}; }
int main(int argc,char** argv) {
    assert(argc==2); const auto command=read(argv[1]); assert(!command.empty());
    char temp[]="/tmp/better-favorites-home-compat.XXXXXX";
    char request[]="/tmp/better-favorites.XXXXXX";
    char context[]="/tmp/better-favorites-return.XXXXXX";
    assert(mkdtemp(temp) && mkdtemp(context) && mkdtemp(request)); const std::string root=temp, ctx=context;
    fs::create_directories(root+"/.tmp_update/bin"); fs::create_directories(root+"/Roms");
    const std::string req=request;
    const auto active=root+"/.tmp_update/cmd_to_run.sh", quick=root+"/quick", pending=root+"/pending", shutdown=root+"/shutdown", settings=root+"/settings";
    write(root+"/.tmp_update/bin/gameSwitcher","#!/bin/sh\n"); assert(chmod((root+"/.tmp_update/bin/gameSwitcher").c_str(),0700)==0);
    setenv("BETTER_FAVORITES_SETTINGS",settings.c_str(),1); setenv("BETTER_FAVORITES_RETURN_DIR",ctx.c_str(),1); setenv("BETTER_FAVORITES_SWITCHER_HANDOFF","1",1);
    std::string error; AppSettings app;
    for(bool on:{false,true}) {
        assert(setAutomaticReturn(settings,on,app,error));
        // Actual emitted native command passes existing A ownership guard.
        write(req+"/app-command.sh",command); write(active,command);
        assert(stageOnionLaunchCommand("game fixture\n","{\"type\":5}\n",req,error));
        assert(publishOnionLaunchCommand(req,active,quick,root,error));
        assert(fs::exists(ctx+"/request.sh")==on); assert(fs::exists(ctx+"/generation")==on);
        assert(cancelOnionLaunchCommand(req,error)); unlink(quick.c_str());
        unlink((ctx+"/request.sh").c_str()); unlink((ctx+"/generation").c_str());
        // Actual command also passes existing MENU guard without registration.
        const auto before=read(root+"/Roms/recentlist-hidden.json");
        write(req+"/app-command.sh",command); write(active,command);
        assert(stageOnionSwitcherRequest(req,error));
        assert(publishOnionSwitcherRequest(req,active,quick,pending,shutdown,root,error));
        assert(fs::exists(ctx+"/switcher.request")==on); assert(fs::exists(ctx+"/generation")==on);
        assert(read(root+"/Roms/recentlist-hidden.json")==before);
        assert(cancelOnionLaunchCommand(req,error)); unlink((root+"/.tmp_update/.runGameSwitcher").c_str());
        unlink((ctx+"/switcher.request").c_str()); unlink((ctx+"/generation").c_str());
    }
    fs::remove_all(root); fs::remove_all(ctx); fs::remove_all(req);
}
