#include "title_scroll.h"
#include <cassert>
#include <cstdint>
#include <iostream>
int main() {
    TitleScroll s;
    auto at=[&](std::uint32_t t,bool paused=false){return s.update("launch/rom#1","Pokémon 日本語",400,250,t,paused);};
    assert(at(100)==0 && at(1099)==0 && at(1100)==0);
    assert(at(1600)==15 && at(2600)==45);
    assert(at(6100)==150 && at(7099)==150); // Full end visible for one second.
    assert(at(7100)==0 && at(8099)==0 && at(8600)==15);
    s.pause(8700);assert(at(8800,true)==18 && at(90000,true)==18);
    assert(at(90001)==0 && at(91000)==0 && at(91501)==15);
    // New selected record with the same text still resets; source span disambiguates duplicates.
    assert(s.update("launch/rom#2","Pokémon 日本語",400,250,92000,false)==0);
    assert(s.update("launch/rom#2","1. Pokémon 日本語",420,250,94000,false)==0);
    assert(s.update("launch/rom#2","1. Pokémon 日本語",420,600,96000,false)==0);
    assert(s.update("launch/rom#2","1. Pokémon 日本語",420,0,98000,false)==0);
    assert(s.update("launch/rom#2","1. Pokémon 日本語",420,250,99000,false)==0);
    assert(s.update("launch/rom#2","1. Pokémon 日本語",420,250,101000,false)==30);
    s.restart(101010);assert(s.update("launch/rom#2","1. Pokémon 日本語",420,250,101500,false)==0);
    TitleScroll wrap;const std::uint32_t start=UINT32_MAX-500;
    assert(wrap.update("x","long",300,200,start,false)==0);
    assert(wrap.update("x","long",300,200,start+1500u,false)==15);
    TitleScroll shortTitle;
    assert(shortTitle.update("x","short",100,100,0,false)==0);
    assert(shortTitle.update("x","short",100,100,1000000,false)==0);
    std::cout<<"Title timing, end hold/restart, selection/label/width reset, menu pause/resume, short/empty region and tick wrap: PASS\n";
}
