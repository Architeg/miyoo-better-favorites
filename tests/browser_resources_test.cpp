#include "browser_resources.h"
#include "theme_loader.h"
#include "theme_fonts.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
namespace fs=std::filesystem;
void write(const fs::path& p,const std::string& data){fs::create_directories(p.parent_path());std::ofstream f(p);f<<data;assert(f.good());}
void image(const fs::path& p,int w,int h,Uint8 red){
    fs::create_directories(p.parent_path());auto* s=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);assert(s);
    SDL_FillRect(s,nullptr,SDL_MapRGBA(s->format,red,60,80,255));assert(IMG_SavePNG(s,p.c_str())==0);SDL_FreeSurface(s);
}
int main(int argc,char** argv){
    assert(argc==2);assert(SDL_Init(0)==0);assert(TTF_Init()==0);assert(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG);
    char temp[]="/tmp/better-favorites-m4-resources.XXXXXX";assert(mkdtemp(temp));fs::path root=temp;
    const auto active=root/"Themes/Active",profile=root/"Saves/CurrentProfile/theme",fallback=root/"miyoo/app";
    write(root/".tmp_update/config/active_theme",active.string());
    write(active/"config.json","{\"title\":{\"font\":\"missing.ttf\",\"size\":24},\"list\":{\"font\":\"bad.ttf\",\"size\":24}}");write(active/"bad.ttf","bad font");
    fs::create_directories(fallback);fs::copy_file(argv[1],fallback/"base.ttf");
    write(fallback/"config.json","{\"title\":{\"font\":\"base.ttf\",\"size\":24},\"list\":{\"font\":\"base.ttf\",\"size\":24},\"hint\":{\"font\":\"base.ttf\",\"size\":20}}");
    auto load=[&](){auto t=ThemeLoader(root.string()).load();resolveThemeFonts(t);return t;};
    image(profile/"skin/background.png",111,100,10);image(active/"skin/background.png",222,100,20);image(fallback/"skin/background.png",333,100,30);
    auto theme=load();auto* s=loadThemeImage(theme,theme.backgroundPath);assert(s && s->w==111);SDL_FreeSurface(s);
    write(profile/"skin/background.png","corrupt PNG");theme=load();s=loadThemeImage(theme,theme.backgroundPath);assert(s && s->w==222);SDL_FreeSurface(s);
    write(active/"skin/background.png","also corrupt");theme=load();s=loadThemeImage(theme,theme.backgroundPath);assert(s && s->w==333);SDL_FreeSurface(s);
    fs::remove(profile/"skin/background.png");fs::remove(active/"skin/background.png");theme=load();s=loadThemeImage(theme,theme.backgroundPath);assert(s && s->w==333);SDL_FreeSurface(s);
    fs::remove(fallback/"skin/background.png");theme=load();assert(!loadThemeImage(theme,theme.backgroundPath));
    // Required background and selection can still be created at the original geometry.
    auto* bg=createThemeBackground(theme);auto* selection=createThemeSelection(theme);assert(bg && bg->w==640 && bg->h==480 && selection && selection->w==640 && selection->h==56);
    assert(*static_cast<Uint32*>(bg->pixels)!=*static_cast<Uint32*>(selection->pixels));SDL_FreeSurface(bg);SDL_FreeSurface(selection);
    for(auto* style:{&theme.title,&theme.list,&theme.section,&theme.hint}){assert(style->fontPath==(fallback/"base.ttf").string());auto* f=TTF_OpenFont(style->fontPath.c_str(),style->size);assert(f);TTF_CloseFont(f);}
    assert(!loadThemeImage(theme,theme.titleBackgroundPath) && !loadThemeImage(theme,theme.previewBackgroundPath));
    // Same-directory, compatible menu material fallback never scans unrelated themes.
    write(profile/"skin/pop-bg.png","corrupt");image(active/"skin/pop-bg.png",80,100,20);theme=load();s=loadThemeImage(theme,theme.dialogPath);assert(s && s->w==80);SDL_FreeSurface(s);
    SDL_Surface* art=nullptr;std::string current;
    const auto valid=root/"Roms/art.png",bad=root/"Roms/bad.png";image(valid,128,160,40);write(bad,"corrupt artwork");
    updateFavoriteArtwork(art,current,valid.string());assert(art && art->w==128);auto* cached=art;
    updateFavoriteArtwork(art,current,valid.string());assert(art==cached);
    updateFavoriteArtwork(art,current,bad.string());assert(!art && current==bad.string());
    updateFavoriteArtwork(art,current,valid.string());assert(art);
    updateFavoriteArtwork(art,current,(root/"missing.png").string());assert(!art);
    updateFavoriteArtwork(art,current,valid.string());assert(art);updateFavoriteArtwork(art,current,"");assert(!art);
    auto small=fitFavoriteArtwork(100,120,250);assert(small.w==100 && small.h==120 && small.y==180);
    for(const auto pair:{std::pair<int,int>{2000,10},{10,2000},{10000,10000},{1,20000}}){auto rect=fitFavoriteArtwork(pair.first,pair.second,250);assert(rect.x>=390 && rect.x+rect.w<=640 && rect.y>=60 && rect.y+rect.h<=420 && rect.w>=1 && rect.h>=1);}
    assert(fitFavoriteArtwork(0,5,250).w==0 && fitFavoriteArtwork(5,5,0).w==0);
    write(active/"config.json","malformed config {");write(profile/"config.json","invalid JSON");
    theme=load();for(auto* style:{&theme.title,&theme.list,&theme.section,&theme.hint}){
        assert(style->fontPath==(fallback/"base.ttf").string());auto* f=TTF_OpenFont(style->fontPath.c_str(),style->size);assert(f);TTF_CloseFont(f);
    }
    fs::remove(root/".tmp_update/config/active_theme");theme=load();assert(theme.rootPath==fallback.string());
    assert(theme.list.fontPath==(fallback/"base.ttf").string());
    fs::remove_all(root);IMG_Quit();TTF_Quit();SDL_Quit();
    std::cout<<"Missing/corrupt profile->active->Miyoo PNG/font resources, theme-derived required surfaces, optional omissions, cached/no-stale artwork and wide/tall bounds: PASS\n";
}
