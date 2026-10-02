#include "browser_titles.h"
#include <algorithm>
#include <iostream>

SDL_Rect browserTitleRegion(int rowY, int rowHeight, int previewWidth) {
    const int right = previewWidth > 0 ? std::min(620,640-previewWidth-8) : 620;
    const int top = std::max(60,rowY), bottom = std::min(420,rowY+rowHeight);
    return {20,top,std::max(0,right-20),std::max(0,bottom-top)};
}
void BrowserTitles::beginFrame() {
    for (auto& item : cache_) item.second.used = false;
}
void BrowserTitles::endFrame() {
    for (auto it=cache_.begin();it!=cache_.end();) {
        if (!it->second.used) {
            if (it->second.surface) SDL_FreeSurface(it->second.surface);
            it=cache_.erase(it);
        } else ++it;
    }
}
void BrowserTitles::clear() {
    for (auto& item : cache_) if (item.second.surface) SDL_FreeSurface(item.second.surface);
    cache_.clear();
}
void BrowserTitles::draw(SDL_Surface* screen, const std::string& text, SDL_Color color,
                         SDL_Rect region, int rowY, int rowHeight, bool selected,
                         const std::string& identity, Uint32 now, bool menuOpen) {
    if (text.empty() || region.w<=0 || region.h<=0) {
        if (selected) scroll_.update(identity,text,0,region.w,now,menuOpen);
        return;
    }
    const Uint32 rgba=(Uint32(color.r)<<24)|(Uint32(color.g)<<16)|(Uint32(color.b)<<8)|color.a;
    auto found=cache_.find({text,rgba});
    if (found==cache_.end()) {
        Entry entry;
        entry.surface=TTF_RenderUTF8_Blended(font_,text.c_str(),color);
        if (!entry.surface) std::cerr<<"Cannot render browser title: "<<TTF_GetError()<<std::endl;
        found=cache_.emplace(std::make_pair(text,rgba),entry).first;
#ifdef BETTER_FAVORITES_TITLE_TESTING
        ++rasterizations_;
#endif
    }
    auto& entry=found->second;
    entry.used=true;
    const int textWidth=entry.surface?entry.surface->w:0;
    const int offset=selected?scroll_.update(identity,text,textWidth,region.w,now,menuOpen):0;
    if (!entry.surface) return; // Cache failure too; no per-frame retry/log loop.
    SDL_Rect previous,clip;
    SDL_GetClipRect(screen,&previous);
    if (!SDL_IntersectRect(&previous,&region,&clip)) return;
    SDL_SetClipRect(screen,&clip);
    SDL_Rect target{region.x-offset,rowY+(rowHeight-entry.surface->h)/2,entry.surface->w,entry.surface->h};
    SDL_BlitSurface(entry.surface,nullptr,screen,&target);
    SDL_SetClipRect(screen,&previous);
}
