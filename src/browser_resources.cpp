#include "browser_resources.h"
#include <SDL_image.h>
#include <algorithm>
#include <iostream>

SDL_Surface* loadThemeImage(const Theme& theme, const std::string& resolvedPath) {
    if(resolvedPath.empty())return nullptr;
    const auto found=theme.imageCandidates.find(resolvedPath);
    const std::vector<std::string> candidates=found==theme.imageCandidates.end()
        ?std::vector<std::string>{resolvedPath}:found->second;
    for(const auto& path:candidates){
        auto* surface=IMG_Load(path.c_str());
        if(surface && surface->w>0 && surface->h>0){
            if(path!=resolvedPath)std::cerr<<"Theme image fallback: "<<resolvedPath<<" -> "<<path<<std::endl;
            return surface;
        }
        if(surface)SDL_FreeSurface(surface);
    }
    std::cerr<<"Theme image unavailable: "<<resolvedPath<<": "<<IMG_GetError()<<std::endl;
    return nullptr;
}
namespace {
SDL_Color opposite(SDL_Color ink) {
    const bool light=(ink.r*299+ink.g*587+ink.b*114)/1000>=128;
    return light?SDL_Color{Uint8(ink.r/12),Uint8(ink.g/12),Uint8(ink.b/12),255}
        :SDL_Color{Uint8(255-(255-ink.r)/12),Uint8(255-(255-ink.g)/12),Uint8(255-(255-ink.b)/12),255};
}
SDL_Surface* solid(int width,int height,SDL_Color color){
    auto* surface=SDL_CreateRGBSurfaceWithFormat(0,width,height,32,SDL_PIXELFORMAT_RGBA32);
    if(surface)SDL_FillRect(surface,nullptr,SDL_MapRGBA(surface->format,color.r,color.g,color.b,255));
    return surface;
}
}
SDL_Surface* createThemeBackground(const Theme& theme) {
    return solid(640,480,opposite({Uint8(theme.list.red),Uint8(theme.list.green),Uint8(theme.list.blue),255}));
}
SDL_Surface* createThemeSelection(const Theme& theme) {
    // Preserve the 640x56 selected-row asset geometry without a bundled palette.
    const SDL_Color ink{Uint8(theme.selectedRed),Uint8(theme.selectedGreen),Uint8(theme.selectedBlue),255};
    const auto base=opposite(ink);
    const SDL_Color surface{Uint8((base.r*80+ink.r*20)/100),Uint8((base.g*80+ink.g*20)/100),Uint8((base.b*80+ink.b*20)/100),255};
    return solid(640,56,surface);
}
void updateFavoriteArtwork(SDL_Surface*& surface,std::string& currentPath,const std::string& newPath) {
    if(newPath==currentPath)return;
    if(surface)SDL_FreeSurface(surface);
    surface=nullptr;currentPath=newPath;
    if(!newPath.empty()){
        surface=IMG_Load(newPath.c_str());
        if(!surface)std::cerr<<"Preview image failed: "<<newPath<<": "<<IMG_GetError()<<std::endl;
    }
}
SDL_Rect fitFavoriteArtwork(int imageWidth,int imageHeight,int previewWidth) {
    if(imageWidth<=0 || imageHeight<=0 || previewWidth<=0)return {0,0,0,0};
    const int pane=std::min(640,previewWidth);
    const double scale=std::min({1.0,double(pane)/imageWidth,360.0/imageHeight});
    const int width=std::max(1,int(imageWidth*scale)),height=std::max(1,int(imageHeight*scale));
    return {640-pane+(pane-width)/2,240-height/2,width,height};
}
