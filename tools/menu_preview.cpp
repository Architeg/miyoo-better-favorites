// Exactly two contact sheets, using production SDL/theme rendering.
#include "menu_renderer.h"
#include "theme_loader.h"
#include "theme_fonts.h"
#include <dirent.h>
#include <SDL_image.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
void testThemeFonts(const std::string& fixture);
void testMenuSecondaryContrast();
static void label(SDL_Surface* screen,TTF_Font* font,const std::string& text,int x,int y,SDL_Color color){
    auto* image=TTF_RenderUTF8_Blended(font,text.c_str(),color);if(image){SDL_Rect dest{x,y,image->w,image->h};SDL_BlitSurface(image,nullptr,screen,&dest);SDL_FreeSurface(image);}
}
static SDL_Surface* render(const char* root,MenuPage page,bool longTitle,bool error,const std::string& tracePath){
    Theme theme=ThemeLoader(root).load();resolveThemeFonts(theme);
    std::ofstream trace(tracePath,std::ios::app);
    trace<<"root="<<theme.rootPath<<" dialog="<<theme.dialogPath<<" divider="<<theme.horizontalDividerPath<<" hideIcons="<<theme.hideIcons<<" hideHints="<<theme.hideHints<<"\n";
    auto* bg=IMG_Load(theme.backgroundPath.c_str());auto* head=IMG_Load(theme.titleBackgroundPath.c_str());
    auto* footer=IMG_Load(theme.footerBackgroundPath.c_str());auto* selected=IMG_Load(theme.listSmallPath.c_str());
    auto* title=TTF_OpenFont(theme.title.fontPath.c_str(),theme.title.size);
    auto* list=TTF_OpenFont(theme.list.fontPath.c_str(),theme.list.size);assert(title&&list);TTF_SetFontStyle(list,TTF_STYLE_BOLD);
    auto* body=TTF_OpenFont(theme.section.fontPath.c_str(),theme.section.size);auto* hint=TTF_OpenFont(theme.hint.fontPath.c_str(),theme.hint.size);assert(body&&hint);
    auto* screen=SDL_CreateRGBSurface(0,640,480,16,0,0,0,0);assert(screen);
    const SDL_Color ink{Uint8(theme.list.red),Uint8(theme.list.green),Uint8(theme.list.blue),255};
    auto underlay=[&](){
        SDL_FillRect(screen,nullptr,SDL_MapRGB(screen->format,theme.list.red/12,theme.list.green/12,theme.list.blue/12));
        SDL_Rect full{0,0,640,480};if(bg)SDL_BlitScaled(bg,nullptr,screen,&full);
        label(screen,title,"Favorites",260,15,ink);label(screen,body,"GB",20,80,ink);
        for(int i=0;i<5;++i)label(screen,list,i==1?"17. The Final Fantasy Legend [US]":"16. Tetris [WOR]",20,120+i*60,ink);
        if(selected){SDL_Rect dest{0,176,640,selected->h};SDL_BlitScaled(selected,nullptr,screen,&dest);}
    };
    {
        const int originalBodyStyle=TTF_GetFontStyle(body),originalHintStyle=TTF_GetFontStyle(hint),originalListStyle=TTF_GetFontStyle(list);
        MenuRenderer renderer(theme,{bg,head,footer,selected,title,list,body,hint});
        renderer.verifyFonts();
        std::string game="22. Final Fantasy Adventure [US]";
        if(longTitle && page==MenuPage::RemoveConfirm){game.clear();for(int i=0;i<12;++i)game+="Pokémon – A very long favorite title (USA) [Rev A] ";}
        const std::string message=error?"Favorites changed externally; removal cancelled.":"";
        underlay();renderer.draw(screen,page,0,true,true,true,game,0,message);
        if(page==MenuPage::ReturnInfo)assert(renderer.pageCount()==1);
        std::vector<Uint8> first(static_cast<Uint8*>(screen->pixels),static_cast<Uint8*>(screen->pixels)+screen->pitch*screen->h);
        underlay();renderer.draw(screen,page,0,true,true,true,game,5000,message);
        assert(std::memcmp(first.data(),screen->pixels,first.size())==0);
        if(longTitle && page==MenuPage::RemoveConfirm){
            assert(renderer.pageCount()>1);renderer.movePage(1);underlay();renderer.draw(screen,page,0,true,true,true,game,5000,message);
            assert(std::memcmp(first.data(),screen->pixels,first.size())!=0);
        }
        if(page==MenuPage::Help && longTitle){renderer.movePage(1);renderer.draw(screen,page,0,true,true,true,game,5000,message);}
        assert(TTF_GetFontStyle(body)==originalBodyStyle && TTF_GetFontStyle(hint)==originalHintStyle && TTF_GetFontStyle(list)==originalListStyle);
        // Automatic return OFF has no contextual note or integration status.
        underlay();renderer.draw(screen,MenuPage::Settings,0,true,false,false,game,5000);
        auto* off=SDL_ConvertSurface(screen,screen->format,0);assert(off);
        underlay();renderer.draw(screen,MenuPage::Settings,0,true,false,true,game,5000);
        assert(std::memcmp(static_cast<Uint8*>(off->pixels)+310*off->pitch,
                           static_cast<Uint8*>(screen->pixels)+310*screen->pitch,110*screen->pitch)==0);
        underlay();renderer.draw(screen,MenuPage::Settings,0,true,true,true,game,5000);
        assert(std::memcmp(static_cast<Uint8*>(off->pixels)+310*off->pitch,
                           static_cast<Uint8*>(screen->pixels)+310*screen->pitch,110*screen->pitch)!=0);
        // Empty edge strips track the reserved panel surface across scrolling and
        // row selection, independently of text and inline badges.
        auto* panelReference=SDL_ConvertSurface(screen,screen->format,0);assert(panelReference);
        SDL_FreeSurface(off);
        // Offscreen-only checks: no extra preview files.
        for(std::size_t row=0;row<5;++row){AppSettings options;options.groupByConsole=false;options.showNumericPrefixes=false;options.sortMode=SortMode::AlphabeticalTitle;
            underlay();renderer.draw(screen,MenuPage::Settings,row,true,false,false,game,5000,"",options);
            for(int y=310;y<420;++y){
                assert(std::memcmp(static_cast<Uint8*>(panelReference->pixels)+y*panelReference->pitch,
                                   static_cast<Uint8*>(screen->pixels)+y*screen->pitch,16)==0);
            }
        }
        SDL_FreeSurface(panelReference);
        AppSettings flatOptions;flatOptions.groupByConsole=false;
        underlay();renderer.draw(screen,MenuPage::Help,0,true,true,true,game,5000,"",flatOptions);
        for(const auto& testPage:{MenuPage::Settings,MenuPage::ReturnInfo}){
            underlay();renderer.draw(screen,testPage,1,true,false,false,game,5000);
        }
        Theme icons=theme;icons.hideIcons=false;
        MenuRenderer iconsRenderer(icons,{bg,head,footer,selected,title,list,body,hint});
        underlay();iconsRenderer.draw(screen,MenuPage::Settings,0,true,true,true,game,0);
        Theme hidden=theme;hidden.hideHints=true;
        MenuRenderer hiddenRenderer(hidden,{bg,head,footer,selected,title,list,body,hint});
        underlay();hiddenRenderer.draw(screen,MenuPage::Help,0,true,true,true,game,0);
        // Hidden and visible footer pixels must differ, even with hideIcons=1.
        auto* hiddenCopy=SDL_ConvertSurface(screen,screen->format,0);assert(hiddenCopy);
        underlay();renderer.draw(screen,MenuPage::Help,0,true,true,true,game,0);
        assert(std::memcmp(static_cast<Uint8*>(hiddenCopy->pixels)+420*hiddenCopy->pitch,
                           static_cast<Uint8*>(screen->pixels)+420*screen->pitch,60*screen->pitch)!=0);
        SDL_FreeSurface(hiddenCopy);
        // Restore requested preview after offscreen checks.
        underlay();renderer.draw(screen,page,0,true,true,true,game,5000,message);
        if(longTitle){renderer.movePage(1);underlay();renderer.draw(screen,page,0,true,true,true,game,5000,message);}
    }
    for(auto* surface:{bg,head,footer,selected})if(surface)SDL_FreeSurface(surface);
    for(auto* font:{title,list,body,hint})TTF_CloseFont(font);
    return screen;
}
static void sheet(const std::vector<SDL_Surface*>& cells,const std::string& path) {
    auto* result=SDL_CreateRGBSurfaceWithFormat(0,1280,960,32,SDL_PIXELFORMAT_RGBA32);assert(result);
    for(std::size_t i=0;i<cells.size();++i){SDL_Rect dest{int(i%2)*640,int(i/2)*480,640,480};SDL_BlitSurface(cells[i],nullptr,result,&dest);SDL_FreeSurface(cells[i]);}
    assert(IMG_SavePNG(result,path.c_str())==0);SDL_FreeSurface(result);
}
int main(int argc,char** argv){
    if(argc!=5 && argc!=6)return 2;
    const bool checksOnly=argc==6 && std::string(argv[5])=="--checks-only";
    if(argc==6 && !checksOnly)return 2;
    SDL_Init(0);assert(TTF_Init()==0);assert(IMG_Init(IMG_INIT_PNG)&IMG_INIT_PNG);
    testThemeFonts(argv[1]);
    testMenuSecondaryContrast();
    const std::string output=argv[4],trace=output+"/assets.txt";
    if(!checksOnly){
    sheet({render(argv[2],MenuPage::Settings,false,false,trace),render(argv[2],MenuPage::ReturnInfo,false,false,trace),
           render(argv[1],MenuPage::Settings,false,false,trace),render(argv[1],MenuPage::ReturnInfo,false,false,trace)},output+"/settings-about-sheet.png");
    sheet({render(argv[2],MenuPage::RemoveConfirm,false,false,trace),render(argv[1],MenuPage::RemoveConfirm,false,false,trace),
           render(argv[2],MenuPage::RemoveConfirm,true,true,trace),render(argv[1],MenuPage::RemoveConfirm,true,true,trace)},output+"/removal-sheet.png");
    }
    for(const auto* fixture:{argv[1],argv[2],argv[3]}){
        for(const auto pageKind:{MenuPage::Settings,MenuPage::ReturnInfo}){auto* page=render(fixture,pageKind,false,false,trace);SDL_FreeSurface(page);}
        auto* page=render(fixture,MenuPage::RemoveConfirm,true,true,trace);SDL_FreeSurface(page);
        page=render(fixture,MenuPage::Help,true,false,trace);SDL_FreeSurface(page);
    }
    IMG_Quit();TTF_Quit();SDL_Quit();
    std::cout<<"Fixed panel across scrolling rows, composited-surface contrast fallback, text/modal bounds, actual font faces/body size, OFF description, owned-font preservation, single-page About, static and explicit title paging: PASS\n";
}
