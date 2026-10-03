#ifndef BETTER_FAVORITES_STARTUP_PROFILE_H
#define BETTER_FAVORITES_STARTUP_PROFILE_H
// Opt-in startup observation only. No file writes, sampling thread or resident helper.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <time.h>
#include <unistd.h>
namespace startup_profile {
inline std::uint64_t now() {
    timespec time{};
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) return 0;
    return std::uint64_t(time.tv_sec)*1000000 + time.tv_nsec/1000;
}
inline long long uptimeMs() {
    FILE* input=std::fopen("/proc/uptime","r");if(!input)return -1;
    double seconds=0;const bool valid=std::fscanf(input,"%lf",&seconds)==1;
    std::fclose(input);return valid && seconds>=0 && seconds<1e10?static_cast<long long>(seconds*1000):-1;
}
struct Metric { std::uint64_t calls=0,total=0; std::set<std::string> identities; };
struct State {
    bool active=false;
    std::uint64_t started=0;
    long long mainUptime=-1;
    std::map<std::string,Metric> metrics;
};
inline State& state() { static State value; return value; }
inline bool active() { return state().active; }
inline void start() {
    auto& value=state();
    const char* option=std::getenv("BETTER_FAVORITES_PROFILE");
    value.active=option && std::strcmp(option,"1")==0;
    if(value.active){value.metrics.clear();value.started=now();value.mainUptime=uptimeMs();}
}
inline void record(const char* name,std::uint64_t duration,const char* identity=nullptr) {
    if(!active())return;
    auto& metric=state().metrics[name];++metric.calls;metric.total+=duration;
    if(identity)metric.identities.insert(identity);
}
class Scope {
    const char* name_;const char* identity_;std::uint64_t started_;bool running_;
public:
    explicit Scope(const char* name,const char* identity=nullptr):name_(name),identity_(identity),
        started_(active()?now():0),running_(active()) {}
    void end() { if(running_){const auto stopped=now();record(name_,stopped>=started_?stopped-started_:0,identity_);running_=false;} }
    ~Scope(){end();}
    Scope(const Scope&)=delete;Scope& operator=(const Scope&)=delete;
};
inline void finish(const char* outcome,FILE* output=stderr) {
    if(!active())return;
    const auto elapsed=now()-state().started;
    std::fprintf(output,"BF_PROFILE {\"schema\":1,\"kind\":\"summary\",\"outcome\":\"%s\",\"main_us\":%llu,\"pid\":%ld,\"ppid\":%ld,\"main_uptime_ms\":%lld,\"frame_uptime_ms\":%lld}\n",
        outcome,(unsigned long long)elapsed,(long)getpid(),(long)getppid(),state().mainUptime,uptimeMs());
    for(const auto& item:state().metrics){const auto& metric=item.second;
        std::fprintf(output,"BF_PROFILE {\"schema\":1,\"kind\":\"phase\",\"name\":\"%s\",\"calls\":%llu,\"total_us\":%llu,\"unique\":%zu}\n",
            item.first.c_str(),(unsigned long long)metric.calls,(unsigned long long)metric.total,metric.identities.size());
    }
    state().active=false;state().metrics.clear();
}
class Session {
public:
    Session(){start();}
    ~Session(){finish("incomplete_startup");}
};
}
#endif
