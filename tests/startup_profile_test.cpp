#include "startup_profile.h"
#include <cassert>
#include <string>
static std::string contents(FILE* file) {
    fflush(file);rewind(file);std::string result;char bytes[1024];
    while(auto size=fread(bytes,1,sizeof(bytes),file))result.append(bytes,size);
    return result;
}
int main() {
    FILE* output=tmpfile();assert(output);
    for(const char* setting:{"", "0", "true", "yes"}) {
        setenv("BETTER_FAVORITES_PROFILE",setting,1);startup_profile::start();
        {startup_profile::Scope scope("disabled");}
        startup_profile::finish("disabled",output);
        assert(!startup_profile::active() && startup_profile::state().metrics.empty());
    }
    assert(contents(output).empty());
    setenv("BETTER_FAVORITES_PROFILE","1",1);startup_profile::start();
    startup_profile::record("decode",7,"same");startup_profile::record("decode",11,"same");
    startup_profile::record("decode",3,"other");
    {startup_profile::Scope scope("bounded");scope.end();scope.end();}
    assert(startup_profile::state().metrics["decode"].calls==3);
    assert(startup_profile::state().metrics["decode"].total==21);
    assert(startup_profile::state().metrics["decode"].identities.size()==2);
    assert(startup_profile::state().metrics["bounded"].calls==1);
    startup_profile::finish("first_presented_frame",output);
    auto text=contents(output);assert(text.find("\"calls\":3,\"total_us\":21,\"unique\":2")!=std::string::npos);
    assert(text.find("first_presented_frame")!=std::string::npos);
    assert(!startup_profile::active() && startup_profile::state().metrics.empty());
    startup_profile::record("after_startup",99);startup_profile::finish("duplicate",output);
    assert(contents(output)==text);
    fclose(output);
}
