// Partial host baseline, not the device's startup, audio or first usable frame.
#include "startup_profile.h"
#include "profiled_sdl.h"
#include "theme_loader.h"
#include "theme_fonts.h"
#include "favorites_parser.h"
#include "browser_resources.h"
#include "menu_renderer.h"
#include "ui_rows.h"
#include <fstream>
#include <iterator>
#include <iostream>
int main(int argc,char** argv) {
    if(argc!=3)return 1;
    const int repetitions=std::atoi(argv[2]);if(repetitions<1 || repetitions>100)return 1;
    setenv("BETTER_FAVORITES_PROFILE","1",1);
    for(int run=0;run<repetitions;++run) {
        startup_profile::Session session;
        std::cerr<<"BF_HOST_RUN "<<run+1<<"\n";
        Theme theme;
        {startup_profile::Scope phase("theme.total");theme=ThemeLoader(argv[1]).load();}
        std::string source;
        {startup_profile::Scope phase("favorites.read");std::ifstream input(std::string(argv[1])+"/Roms/favourite.json");
            if(!input)return 1;source.assign(std::istreambuf_iterator<char>(input),{});}
        auto favorites=FavoritesParser(argv[1]).loadFavoritesFromText(source);
        startup_profile::Scope grouping("favorites.group_rows");
        auto groups=FavoritesParser(argv[1]).groupFavorites(favorites);
        auto rows=buildUiRows(groups);grouping.end();
        std::cerr<<"BF_HOST_CORPUS favorites="<<favorites.size()<<" bytes="<<source.size()<<"\n";
        if(TTF_Init()!=0)return 1;
        {startup_profile::Scope phase("font.validation");resolveThemeFonts(theme);}
        auto* background=loadThemeImage(theme,theme.backgroundPath);
        auto* titleBackground=loadThemeImage(theme,theme.titleBackgroundPath);
        auto* footer=loadThemeImage(theme,theme.footerBackgroundPath);
        auto* selected=loadThemeImage(theme,theme.listSmallPath);
        std::vector<SDL_Surface*> additional;
        for(const auto& path:{theme.previewBackgroundPath,theme.buttonAPath,theme.buttonBPath,theme.horizontalDividerPath})
            if(!path.empty())additional.push_back(loadThemeImage(theme,path));
        auto* title=profiledFontOpen(theme.title.fontPath.c_str(),theme.title.size);
        auto* list=profiledFontOpen(theme.list.fontPath.c_str(),theme.list.size);
        auto* body=profiledFontOpen(theme.section.fontPath.c_str(),theme.section.size);
        auto* hint=profiledFontOpen(theme.hint.fontPath.c_str(),theme.hint.size);
        if(!background || !title || !list || !body || !hint)return 1;
        {startup_profile::Scope phase("menu.resources");MenuRenderer menu(theme,{background,titleBackground,footer,selected,title,list,body,hint});
            phase.end();startup_profile::finish("host_partial_loaders");}
        for(auto* font:{title,list,body,hint})TTF_CloseFont(font);
        for(auto* image:additional)if(image)SDL_FreeSurface(image);
        for(auto* image:{background,titleBackground,footer,selected})if(image)SDL_FreeSurface(image);
        TTF_Quit();
    }
}
