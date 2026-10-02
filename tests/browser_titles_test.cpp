#include "browser_titles.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

std::vector<Uint8> pixels(SDL_Surface* s) {
    auto* p=static_cast<Uint8*>(s->pixels);return {p,p+s->pitch*s->h};
}
int main(int argc,char** argv) {
    assert(argc==2);assert(SDL_Init(0)==0 && TTF_Init()==0);
    auto* font=TTF_OpenFont(argv[1],24);assert(font);
    const int style=TTF_GetFontStyle(font);
    auto* screen=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_RGBA32);assert(screen);
    const SDL_Color ink{220,220,200,255};
    const auto base=SDL_MapRGBA(screen->format,20,40,60,255);
    const std::string title="01. Pokémon 日本語 — A long game title with an overflowing ending";
    SDL_Rect inherited{8,70,560,330};SDL_SetClipRect(screen,&inherited);
    SDL_Rect region=browserTitleRegion(120,60,250);assert(region.x==20 && region.w==362 && region.y==120 && region.h==60);
    assert(browserTitleRegion(120,60,0).w==600); // Missing preview asset means no artwork drawn.
    assert(browserTitleRegion(400,60,250).h==20);
    assert(browserTitleRegion(30,60,250).y==60);
    assert(browserTitleRegion(120,60,700).w==0);
    {
        BrowserTitles renderer(font);
        auto draw=[&](Uint32 ticks,bool selected=true,bool menu=false){
            SDL_SetClipRect(screen,nullptr);SDL_FillRect(screen,nullptr,base);SDL_SetClipRect(screen,&inherited);
            renderer.beginFrame();renderer.draw(screen,title,ink,region,120,60,selected,"record",ticks,menu);renderer.endFrame();
            SDL_Rect restored;SDL_GetClipRect(screen,&restored);assert(std::memcmp(&restored,&inherited,sizeof inherited)==0);
        };
        draw(0);auto initial=pixels(screen);
        draw(999);assert(pixels(screen)==initial && renderer.rasterizations()==1);
        draw(2000);auto moved=pixels(screen);assert(moved!=initial);
        // Compare with one full UTF-8 raster shifted by 30px; never byte-slice text.
        auto* expected=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_RGBA32);assert(expected);
        SDL_FillRect(expected,nullptr,base);SDL_SetClipRect(expected,&region);
        auto* full=TTF_RenderUTF8_Blended(font,title.c_str(),ink);assert(full);
        SDL_Rect target{region.x-30,120+(60-full->h)/2,full->w,full->h};
        SDL_BlitSurface(full,nullptr,expected,&target);assert(pixels(expected)==moved);
        SDL_FreeSurface(full);SDL_FreeSurface(expected);
        // Every pixel outside the actual title rectangle is untouched.
        for(int y=0;y<480;++y)for(int x=0;x<640;++x)if(x<20||x>=382||y<120||y>=180){
            Uint32 pixel;std::memcpy(&pixel,static_cast<Uint8*>(screen->pixels)+y*screen->pitch+x*4,4);assert(pixel==base);
        }
        renderer.pause(2000);draw(9000,true,true);assert(pixels(screen)==moved);
        draw(10000);assert(pixels(screen)==initial);draw(10999);assert(pixels(screen)==initial);
        draw(12000,false);assert(pixels(screen)==initial); // Unselected titles never animate.
        for(Uint32 tick=13000;tick<29000;tick+=16)draw(tick);
        assert(renderer.rasterizations()==1 && renderer.cachedCount()==1); // No idle per-frame text rasterization.
        for(int i=0;i<100;++i){renderer.beginFrame();renderer.draw(screen,"Row "+std::to_string(i),ink,region,120,60,true,std::to_string(i),30000+i,false);renderer.endFrame();assert(renderer.cachedCount()==1);}
        renderer.beginFrame();renderer.endFrame();assert(renderer.cachedCount()==0);
        renderer.clear();assert(TTF_GetFontStyle(font)==style);
    }
    SDL_FreeSurface(screen);TTF_CloseFont(font);TTF_Quit();SDL_Quit();
    std::cout<<"UTF-8 full-raster pixels, preview/content/inherited clipping, static rows, pause/resume, bounded cache reuse/eviction and borrowed font: PASS\n";
}
