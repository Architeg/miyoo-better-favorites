#include "theme_loader.h"
#include "theme_fonts.h"
#include "menu_renderer.h"
#include <SDL_ttf.h>
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>
namespace fs=std::filesystem;
namespace {
void write(const fs::path& path,const std::string& text){fs::create_directories(path.parent_path());std::ofstream file(path);file<<text;assert(file.good());}
std::string config(const std::string& font){return "{\"title\":{\"font\":\""+font+"\",\"size\":24},\"list\":{\"font\":\""+font+"\",\"size\":24},\"hint\":{\"font\":\""+font+"\",\"size\":20}}";}
bool regular(TTF_Font* font){
    if(!font || (TTF_GetFontStyle(font)&(TTF_STYLE_BOLD|TTF_STYLE_ITALIC)))return false;
    std::string style=TTF_FontFaceStyleName(font)?TTF_FontFaceStyleName(font):"";
    std::transform(style.begin(),style.end(),style.begin(),[](unsigned char c){return std::tolower(c);});
    return style=="regular"||style=="normal"||style=="book"||style=="roman";
}
}
void testThemeFonts(const std::string& fixture){
    auto supplied=ThemeLoader(fixture).load();resolveThemeFonts(supplied);
    auto* face=TTF_OpenFont(supplied.list.fontPath.c_str(),24);assert(regular(face));
    const std::string family=TTF_FontFaceFamilyName(face);TTF_CloseFont(face);
    fs::path heavy;
    for(const auto& entry:fs::directory_iterator(supplied.rootPath)){
        if(entry.path().extension()!=".ttf"&&entry.path().extension()!=".otf")continue;
        auto* font=TTF_OpenFont(entry.path().c_str(),24);if(!font)continue;
        if(!regular(font)&&family==TTF_FontFaceFamilyName(font))heavy=entry.path();
        TTF_CloseFont(font);
    }
    assert(!heavy.empty()); // The existing light fixture supplies both faces.
    char temp[]="/tmp/better-favorites-font-test.XXXXXX";assert(mkdtemp(temp));const fs::path root=temp;
    const auto active=root/"Themes/Active",profile=root/"Saves/CurrentProfile/theme",fallback=root/"miyoo/app",other=root/"Themes/Unrelated";
    for(const auto& directory:{active,profile,fallback,other})fs::create_directories(directory);
    write(root/".tmp_update/config/active_theme",active.string()+"\n");
    write(fallback/"config.json",config("base.ttf"));
    write(active/"config.json",config("active.ttf"));
    write(profile/"config.json","{\"list\":{\"font\":\"profile.ttf\"}}");
    for(const auto& path:{fallback/"base.ttf",active/"active.ttf",profile/"profile.ttf"})fs::copy_file(supplied.list.fontPath,path);
    auto load=[&](){auto theme=ThemeLoader(root.string()).load();resolveThemeFonts(theme);return theme;};
    auto theme=load();assert(theme.list.fontPath==(profile/"profile.ttf").string()&&theme.list.size==24);
    write(profile/"profile.ttf","not a usable font");theme=load();assert(theme.list.fontPath==(active/"active.ttf").string());
    fs::remove(profile/"profile.ttf");theme=load();assert(theme.list.fontPath==(active/"active.ttf").string());
    fs::remove(active/"active.ttf");theme=load();assert(theme.list.fontPath==(fallback/"base.ttf").string());
    // A valid heavy theme face is not replaced by the regular system fallback.
    fs::copy_file(heavy,active/"active.ttf");write(profile/"config.json","{}");
    fs::copy_file(supplied.list.fontPath,other/"regular.ttf");
    auto check=[&](bool wanted){
        theme=load();assert(theme.list.fontPath==(active/"active.ttf").string());
        auto* font=TTF_OpenFont(theme.list.fontPath.c_str(),24);assert(font);
        {MenuRenderer renderer(theme,{nullptr,nullptr,nullptr,nullptr,font,font,font,font});renderer.verifyFonts(wanted);assert(renderer.explanationsAreRegular()==wanted);}
        TTF_CloseFont(font);
    };
    check(false); // Neither system fallback nor unrelated installed theme is a weight source.
    fs::copy_file(supplied.list.fontPath,active/"regular.ttf");check(true);
    fs::remove(active/"regular.ttf");fs::copy_file(supplied.list.fontPath,profile/"regular.ttf");check(true);
    fs::remove(profile/"regular.ttf");check(false);
    fs::remove(root/".tmp_update/config/active_theme");theme=load();assert(theme.list.fontPath==(fallback/"base.ttf").string());
    fs::remove_all(root); // Only this test's private fixture, never historical files.
    std::cout<<"Font profile/active/system precedence, missing/corrupt fallback, same-family regular scope, heavy-face retention and size preservation: PASS\n";
}
