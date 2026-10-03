#include "diagnostics.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
std::string read(const std::string& p){std::ifstream f(p);return {std::istreambuf_iterator<char>(f),{}};}
int main(){char tmp[]="/tmp/bf-log.XXXXXX";assert(mkdtemp(tmp));std::string p=std::string(tmp)+"/better-favorites.log";setenv("BETTER_FAVORITES_LOG",p.c_str(),1);diagnostics::rotate();{diagnostics::Streams stream;std::cerr<<"complete "<<"line"<<std::endl;}assert(read(p)=="complete line\n");for(int i=0;i<10000;i++)diagnostics::event(std::string(1000,'a'));assert(read(p).size()<=65536);auto before=read(p);diagnostics::rotate();assert(read(std::string(tmp)+"/better-favorites.previous.log")==before);assert(read(p).empty());unlink(p.c_str());assert(mkfifo(p.c_str(),0600)==0);diagnostics::rotate();diagnostics::event("must not block");unlink(p.c_str());assert(symlink("foreign",p.c_str())==0);diagnostics::event("do not follow");unlink(p.c_str());unlink((std::string(tmp)+"/better-favorites.previous.log").c_str());rmdir(tmp);std::cout<<"Bounded logging/rotation/nonregular/failure fixtures: PASS\n";}
