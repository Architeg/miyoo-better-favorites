#include "theme_fonts.h"
#include <SDL_ttf.h>
#include <algorithm>
#include <iostream>

void resolveThemeFonts(Theme& theme) {
    for(auto* style:{&theme.title,&theme.list,&theme.hint,&theme.section}) {
        auto candidates=style->fontCandidates;
        if(candidates.empty())candidates.push_back(style->fontPath);
        for(const auto& path:candidates) {
            auto* font=TTF_OpenFont(path.c_str(),std::max(1,style->size));
            if(font){TTF_CloseFont(font);style->fontPath=path;break;}
            std::cerr<<"Cannot open theme font: "<<path<<": "<<TTF_GetError()<<std::endl;
        }
    }
}
