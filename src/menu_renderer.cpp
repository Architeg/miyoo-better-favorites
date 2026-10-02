#include "menu_renderer.h"
#include "menu_text.h"
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <fstream>
#include <cctype>
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
#include <cassert>
#endif
namespace {
constexpr int margin = 20, header = 60, bottom = 420, rowHeight = 60;
SDL_Color color(const ThemeTextStyle& style) { return {Uint8(style.red), Uint8(style.green), Uint8(style.blue), 255}; }
SDL_Color mix(SDL_Color a, SDL_Color b, int percent) {
    return {Uint8((a.r*(100-percent)+b.r*percent)/100), Uint8((a.g*(100-percent)+b.g*percent)/100),
            Uint8((a.b*(100-percent)+b.b*percent)/100), 255};
}
int luminance(SDL_Color c) {return (c.r*299+c.g*587+c.b*114)/1000;}
SDL_Color contrastBase(SDL_Color c) {
    if(luminance(c)>=128)return {Uint8(c.r/12),Uint8(c.g/12),Uint8(c.b/12),255};
    return {Uint8(255-(255-c.r)/12),Uint8(255-(255-c.g)/12),Uint8(255-(255-c.b)/12),255};
}
SDL_Color popupColor(SDL_Color bg,SDL_Color ink) {
    if(luminance(ink)<128)return mix(bg,ink,20);
    return mix(bg,ink,12);
}
SDL_Color sample(SDL_Surface* surface, SDL_Color fallback) {
    if (!surface) return fallback;
    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!rgba) return fallback;
    unsigned long long red=0, green=0, blue=0, weight=0;
    SDL_LockSurface(rgba);
    for (int y=0; y<rgba->h; y+=4) for (int x=0; x<rgba->w; x+=4) {
        auto pixel = *reinterpret_cast<Uint32*>(static_cast<Uint8*>(rgba->pixels)+y*rgba->pitch+x*4);
        Uint8 r,g,b,a; SDL_GetRGBA(pixel, rgba->format, &r,&g,&b,&a);
        red+=r*a; green+=g*a; blue+=b*a; weight+=a;
    }
    SDL_UnlockSurface(rgba); SDL_FreeSurface(rgba);
    return weight ? SDL_Color {Uint8(red/weight),Uint8(green/weight),Uint8(blue/weight),255} : fallback;
}
void fill(SDL_Surface* screen, SDL_Rect rect, SDL_Color c) { SDL_FillRect(screen,&rect,SDL_MapRGB(screen->format,c.r,c.g,c.b)); }
void blit(SDL_Surface* image, SDL_Surface* screen, SDL_Rect rect) { if (image) SDL_BlitScaled(image,nullptr,screen,&rect); }
int width(TTF_Font* font, const std::string& text) { int w=0,h=0; if(font) TTF_SizeUTF8(font,text.c_str(),&w,&h); return w; }
int lineHeight(TTF_Font* font) { return font ? TTF_FontLineSkip(font) : 24; }
std::vector<std::string> wrap(TTF_Font* font, const std::string& text, int w) {
    return wrapMenuText(text,w,[&](const std::string& s){return width(font,s);});
}
void text(SDL_Surface* screen, TTF_Font* font, const std::string& value, SDL_Color c, int x, int y) {
    if (!font || value.empty()) return;
    SDL_Surface* rendered = TTF_RenderUTF8_Blended(font,value.c_str(),c);
    if (!rendered) return;
    SDL_Rect rect {x,y,rendered->w,rendered->h};
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
    assert(x >= 0 && y >= 0 && x+rendered->w <= screen->w && y+rendered->h <= screen->h);
#endif
    SDL_BlitSurface(rendered,nullptr,screen,&rect); SDL_FreeSurface(rendered);
}
void centered(SDL_Surface* screen, TTF_Font* font, const std::string& value, SDL_Color c, SDL_Rect rect) {
    text(screen,font,value,c,rect.x+(rect.w-width(font,value))/2,rect.y+(rect.h-TTF_FontHeight(font))/2);
}
void boundary(SDL_Surface* screen, SDL_Rect rect, SDL_Color c) {
    fill(screen,{rect.x,rect.y,rect.w,1},c); fill(screen,{rect.x,rect.y+rect.h-1,rect.w,1},c);
    fill(screen,{rect.x,rect.y,1,rect.h},c); fill(screen,{rect.x+rect.w-1,rect.y,1,rect.h},c);
}
int control(SDL_Surface* screen, SDL_Surface* label, int x, int centerY) {
    if(!label)return 0;
    SDL_Rect rect{x,centerY-label->h/2,label->w,label->h};
    SDL_BlitSurface(label,nullptr,screen,&rect);return label->w;
}
TTF_Font* readableFont(const ThemeTextStyle& style,int maxHeight=0) {
    std::string path=style.fontPath;
    const auto dot=path.find_last_of('.');
    std::string stem=dot==std::string::npos?path:path.substr(0,dot);
    for(const std::string suffix:{"-Regular","_Regular"}) {
        if(stem.size()>=suffix.size() && stem.compare(stem.size()-suffix.size(),suffix.size(),suffix)==0){stem.resize(stem.size()-suffix.size());break;}
    }
    // Only a heavier face in the same family/directory is eligible.
    for(const auto* suffix:{"-SemiBold.otf","-SemiBold.ttf","-Bold.otf","-Bold.ttf"}) {
        const auto candidate=stem+suffix;
        if(std::ifstream(candidate).good()){path=candidate;break;}
    }
    for(int size=style.size+1;size>=8;--size) {
        auto* font=TTF_OpenFont(path.c_str(),size);if(!font)return nullptr;
        std::string face=TTF_FontFaceStyleName(font)?TTF_FontFaceStyleName(font):"";
        std::transform(face.begin(),face.end(),face.begin(),[](unsigned char c){return std::tolower(c);});
        if(face.find("bold")==std::string::npos)TTF_SetFontStyle(font,TTF_STYLE_BOLD);
        if(!maxHeight || TTF_FontHeight(font)<=maxHeight)return font;
        TTF_CloseFont(font);
    }
    return nullptr;
}
void arrow(SDL_Surface* screen,TTF_Font* font,const std::string& key,SDL_Color ink,SDL_Rect rect) {
    const Uint16 glyph=key=="UP"?0x2191:key=="DOWN"?0x2193:key=="LEFT"?0x2190:0x2192;
    if(TTF_GlyphIsProvided(font,glyph)) {
        const char* value=key=="UP"?"\xe2\x86\x91":key=="DOWN"?"\xe2\x86\x93":key=="LEFT"?"\xe2\x86\x90":"\xe2\x86\x92";
        centered(screen,font,value,ink,rect);
        // A one-pixel second pass strengthens a thin font arrow without changing layout.
        rect.x+=1;centered(screen,font,value,ink,rect);return;
    }
    const int cx=rect.x+rect.w/2, cy=rect.y+rect.h/2;
    const bool vertical=key=="UP"||key=="DOWN";
    const int sign=key=="UP"||key=="LEFT"?-1:1;
    if(vertical)fill(screen,{cx-1,cy-9,3,18},ink);else fill(screen,{cx-9,cy-1,18,3},ink);
    for(int i=0;i<7;++i) {
        if(vertical) {fill(screen,{cx-i,cy+sign*(9-i),3,3},ink);fill(screen,{cx+i-1,cy+sign*(9-i),3,3},ink);}
        else {fill(screen,{cx+sign*(9-i),cy-i,3,3},ink);fill(screen,{cx+sign*(9-i),cy+i-1,3,3},ink);}
    }
}
struct Painter {
    SDL_Surface* screen; const Theme& theme; MenuResources resources;
    SDL_Color list, selected, hint, background, panel;
    std::function<SDL_Surface*(const std::string&)> label;
    int row(const std::string& label,const std::string& value,int y,int w,bool active,bool disabled=false) {
        auto font=resources.listFont;
        const int valueWidth=value.empty()?0:width(font,value)+24;
        const auto lines=wrap(font,label,w-margin*2-valueWidth);
        const int height=std::max(rowHeight,int(lines.size())*lineHeight(font)+16);
        if(active) {
            if(resources.selection) blit(resources.selection,screen,{0,y+(height-resources.selection->h)/2,w,resources.selection->h});
            else fill(screen,{0,y,w,height},mix(panel,list,18));
        }
        SDL_Color c=active?selected:list; if(disabled)c=mix(c,panel,60);
        int top=y+(height-int(lines.size())*lineHeight(font))/2;
        for(const auto& line:lines) {text(screen,font,line,c,margin,top);top+=lineHeight(font);}
        if(!value.empty()) text(screen,font,value,c,w-margin-width(font,value),y+(height-TTF_FontHeight(font))/2);
        return height;
    }
    int paragraph(const std::string& value,int y,int w=600) {
        auto lines=wrap(resources.bodyFont,value,w);
        for(const auto& line:lines) {text(screen,resources.bodyFont,line,list,margin,y);y+=lineHeight(resources.bodyFont)+4;}
        return y;
    }
    void footer(const std::vector<std::pair<std::string,std::string>>& items) {
        fill(screen,{0,bottom,640,60},background); blit(resources.footer,screen,{0,bottom,640,60});
        hints(items,margin,450);
    }
    void hints(const std::vector<std::pair<std::string,std::string>>& items,int x,int center) {
        if(theme.hideHints)return;
        for(const auto& item:items) {
            x+=control(screen,label(item.first),x,center)+10;
            text(screen,resources.hintFont,item.second,hint,x,center-TTF_FontHeight(resources.hintFont)/2);
            x+=width(resources.hintFont,item.second)+28;
        }
    }
};
}
MenuRenderer::MenuRenderer(const Theme& theme,MenuResources resources):theme_(theme),resources_(resources) {
    if(!theme.actionMenuPath.empty()) popup_=IMG_Load(theme.actionMenuPath.c_str());
    if(!theme.menuLeftArrowPath.empty()) leftArrow_=IMG_Load(theme.menuLeftArrowPath.c_str());
    if(!theme.menuRightArrowPath.empty()) rightArrow_=IMG_Load(theme.menuRightArrowPath.c_str());
    // App-owned instances only. Browser resources are borrowed and never restyled.
    ownedBodyFont_=readableFont(theme.section);
    ownedHintFont_=readableFont(theme.hint,26);
    if(ownedBodyFont_)resources_.bodyFont=ownedBodyFont_;
    if(ownedHintFont_)resources_.hintFont=ownedHintFont_;
    const auto ink=color(theme_.list);
    backgroundColor_=sample(resources_.background,contrastBase(ink));
    panelColor_=popupColor(backgroundColor_,ink);
    if(!theme.dialogPath.empty()) {
        auto* asset=IMG_Load(theme.dialogPath.c_str());
        // Some themes bake CANCEL/OK into the dialog footer. Only reuse the
        // interior material; our measured text controls remain authoritative.
        if(asset) {
            SDL_Rect interior{asset->w/4,asset->h/4,std::max(1,asset->w/2),std::max(1,asset->h/4)};
            dialog_=SDL_CreateRGBSurfaceWithFormat(0,interior.w,interior.h,32,SDL_PIXELFORMAT_RGBA32);
            if(dialog_){SDL_FillRect(dialog_,nullptr,SDL_MapRGBA(dialog_->format,0,0,0,0));SDL_BlitSurface(asset,&interior,dialog_,nullptr);}
            SDL_FreeSurface(asset);
        }
    }
    const auto popupInk=sample(popup_,panelColor_);
    usePopup_=popup_ && std::abs(luminance(popupInk)-luminance(backgroundColor_))>=12 && std::abs(luminance(popupInk)-luminance(ink))>=80;
    if(!theme.horizontalDividerPath.empty())divider_=IMG_Load(theme.horizontalDividerPath.c_str());
    shade_=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_RGBA32);
    if(shade_){SDL_FillRect(shade_,nullptr,SDL_MapRGBA(shade_->format,backgroundColor_.r,backgroundColor_.g,backgroundColor_.b,170));SDL_SetSurfaceBlendMode(shade_,SDL_BLENDMODE_BLEND);}
}
SDL_Surface* MenuRenderer::controlLabel(const std::string& key) {
    auto found=controls_.find(key);if(found!=controls_.end())return found->second;
    auto* font=resources_.hintFont;const auto ink=color(theme_.hint);
    const int h=std::max(28,TTF_FontHeight(font)+6);
    const bool direction=key=="UP"||key=="DOWN"||key=="LEFT"||key=="RIGHT"||key=="UP DOWN"||key=="LEFT RIGHT";
    const bool face=key.size()==1 && std::string("ABXY").find(key)!=std::string::npos;
    const int w=direction?(key.find(' ')==std::string::npos?h:h*2+8):face?h:width(font,key)+16;
    auto* surface=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);
    if(!surface)return nullptr;
    SDL_FillRect(surface,nullptr,SDL_MapRGBA(surface->format,0,0,0,0));
    SDL_SetSurfaceBlendMode(surface,SDL_BLENDMODE_BLEND);
    if(direction) {
        std::istringstream parts(key);std::string part;int x=0;
        while(parts>>part){
            auto* asset=part=="LEFT"?leftArrow_:part=="RIGHT"?rightArrow_:nullptr;
            if(asset && !theme_.hideIcons)blit(asset,surface,{x+(h-24)/2,(h-24)/2,24,24});
            else arrow(surface,font,part,ink,{x,0,h,h});
            x+=h+8;
        }
    } else {
        // Filled text-control circle/capsule using only resolved theme colors.
        const int radius=h/2;
        for(int y=0;y<h;++y){const double dy=y+0.5-radius;const int inset=radius-int(std::sqrt(std::max(0.0,double(radius*radius)-dy*dy)));fill(surface,{inset,y,w-2*inset,1},ink);}
        const auto letter=std::abs(luminance(backgroundColor_)-luminance(ink))>=80?backgroundColor_:contrastBase(ink);
        centered(surface,font,key,letter,{0,0,w,h});
    }
    controls_[key]=surface;return surface;
}
void MenuRenderer::release(){
    for(auto& entry:controls_)SDL_FreeSurface(entry.second);
    controls_.clear();
    for(auto* surface:{dialog_,divider_,shade_})if(surface)SDL_FreeSurface(surface);
    dialog_=divider_=shade_=nullptr;
    if(popup_) SDL_FreeSurface(popup_);
    popup_=nullptr;
    if(leftArrow_) SDL_FreeSurface(leftArrow_);
    if(rightArrow_) SDL_FreeSurface(rightArrow_);
    leftArrow_=rightArrow_=nullptr;
    if(ownedBodyFont_)TTF_CloseFont(ownedBodyFont_);
    if(ownedHintFont_)TTF_CloseFont(ownedHintFont_);
    ownedBodyFont_=ownedHintFont_=nullptr;
}
MenuRenderer::~MenuRenderer(){release();}
void MenuRenderer::movePage(int direction){page_=std::max(0,std::min(pages_-1,page_+direction));}
void MenuRenderer::draw(SDL_Surface* screen,MenuPage page,std::size_t selected,bool hasFavorite,
                        bool returnOn,bool available,const std::string& gameTitle,Uint32 ticks,const std::string& error) {
    (void)ticks;
    if(previous_!=page){page_=0;pages_=1;previous_=page;}
    const auto list=color(theme_.list),hint=color(theme_.hint);
    const SDL_Color selection{Uint8(theme_.selectedRed),Uint8(theme_.selectedGreen),Uint8(theme_.selectedBlue),255};
    const auto bg=backgroundColor_;
    const auto surface=panelColor_;
    Painter p{screen,theme_,resources_,list,selection,hint,bg,surface,[&](const std::string& key){return controlLabel(key);}};
    if(page==MenuPage::Actions) {
        const char* labels[]={"Launch","Remove from Favorites","Settings","Help"};
        int w=320;
        for(const auto* label:labels)w=std::min(620,std::max(w,width(resources_.listFont,label)+40));
        int height=0;
        for(const auto* label:labels)height+=std::max(60,int(wrap(resources_.listFont,label,w-40).size())*lineHeight(resources_.listFont)+16);
        fill(screen,{0,0,w,height},surface);
        if(usePopup_)
            blit(popup_,screen,{0,0,w,height});
        int y=0;for(std::size_t i=0;i<4;++i)y+=p.row(labels[i],"",y,w,selected==i,i<2&&!hasFavorite);
        boundary(screen,{0,0,w,height},mix(surface,list,20));
        p.footer({{"A","Choose"},{"B","Back"}});return;
    }
    if(page==MenuPage::RemoveConfirm) {
        blit(shade_,screen,{0,0,640,480});
        const int inset=20;
        const int panelWidth=std::min(600,std::max({400,
            width(resources_.titleFont,"Remove from Favorites?")+inset*2,
            width(resources_.listFont,gameTitle)+inset*2,
            width(resources_.bodyFont,"The game file will be kept.")+inset*2,
            error.empty()?0:width(resources_.bodyFont,conciseMenuError(error))+inset*2}));
        const int inner=panelWidth-inset*2;
        const int titleH=lineHeight(resources_.titleFont), textH=lineHeight(resources_.bodyFont);
        const int choiceH=std::max(36,TTF_FontHeight(resources_.listFont)+8);
        const auto heading=wrap(resources_.titleFont,"Remove from Favorites?",inner);
        const auto lines=wrap(resources_.listFont,gameTitle.empty()?"No game selected":gameTitle,inner);
        const auto note=wrap(resources_.bodyFont,"The game file will be kept.",inner);
        const auto errors=error.empty()?std::vector<std::string>{}:wrap(resources_.bodyFont,conciseMenuError(error),inner);
        const int base=32+int(heading.size())*titleH+12+int(note.size())*textH+12+choiceH*2+44+(errors.empty()?0:int(errors.size())*textH+12);
        const int lineH=lineHeight(resources_.listFont);
        int perPage=std::max(1,std::min(3,(432-base)/lineH));
        pages_=std::max(1,(int(lines.size())+perPage-1)/perPage);
        if(pages_>1){perPage=std::max(1,std::min(3,(432-base-36)/lineH));pages_=std::max(1,(int(lines.size())+perPage-1)/perPage);}
        page_=std::min(page_,pages_-1);
        const int visible=std::min(perPage,int(lines.size())-page_*perPage);
        const int panelHeight=base+visible*lineH+(pages_>1?36:0);
        const int x=(640-panelWidth)/2, top=(480-panelHeight)/2;
        SDL_Rect panel{x,top,panelWidth,panelHeight};fill(screen,panel,surface);
        if(dialog_)blit(dialog_,screen,panel);
        boundary(screen,panel,mix(surface,list,22));
        int y=top+16;
        for(const auto& line:heading){text(screen,resources_.titleFont,line,color(theme_.title),x+inset,y);y+=titleH;}
        y+=12;
        for(int i=page_*perPage;i<page_*perPage+visible;++i){text(screen,resources_.listFont,lines[i],list,x+inset,y);y+=lineH;}
        if(pages_>1){
            y+=4;fill(screen,{x+inset,y,panelWidth-inset*2,1},mix(surface,list,20));y+=4;
            text(screen,resources_.hintFont,std::to_string(page_+1)+" of "+std::to_string(pages_),hint,x+inset,y);
            // Paging is informative content, not a footer; the control remains visible.
            auto* label=controlLabel("LEFT RIGHT");control(screen,label,x+panelWidth-inset-(label?label->w:0)-width(resources_.hintFont,"Page")-10,y+TTF_FontHeight(resources_.hintFont)/2);
            text(screen,resources_.hintFont,"Page",hint,x+panelWidth-inset-width(resources_.hintFont,"Page"),y);
            y+=28;
        }
        for(const auto& line:note){text(screen,resources_.bodyFont,line,list,x+inset,y);y+=textH;}
        y+=12;
        for(const auto& line:errors){text(screen,resources_.bodyFont,line,list,x+inset,y);y+=textH;}
        if(!errors.empty())y+=12;
        for(std::size_t i=0;i<2;++i){
            if(selected==i){
                if(resources_.selection)blit(resources_.selection,screen,{x+1,y,panelWidth-2,choiceH});
                else fill(screen,{x+1,y,panelWidth-2,choiceH},mix(surface,list,18));
            }
            text(screen,resources_.listFont,i==0?"Cancel":"Remove",selected==i?selection:list,x+inset,y+(choiceH-TTF_FontHeight(resources_.listFont))/2);y+=choiceH;
        }
        p.hints({{"A",selected==0?"Cancel":"Remove"},{"B","Back"}},x+inset,top+panelHeight-22);
        return;
    }
    fill(screen,{0,0,640,480},bg);blit(resources_.background,screen,{0,0,640,480});blit(resources_.title,screen,{0,0,640,header});
    const std::string title=page==MenuPage::Settings?"SETTINGS":page==MenuPage::Help?"HELP":
        page==MenuPage::ReturnInfo?"AUTOMATIC RETURN":"REMOVE FROM FAVORITES?";
    const auto titleLines=wrap(resources_.titleFont,title,600);
    // Titles are short fixed labels, measured and wrapped rather than clipped.
    int headingY=(header-int(titleLines.size())*lineHeight(resources_.titleFont))/2;
    for(const auto& line:titleLines){text(screen,resources_.titleFont,line,color(theme_.title),(640-width(resources_.titleFont,line))/2,headingY);headingY+=lineHeight(resources_.titleFont);}
    if(page==MenuPage::Settings) {
        int y=header;
        y+=p.row("Automatic return",returnOn?"ON":"OFF",y,640,selected==0);
        y+=p.row("About automatic return","",y,640,selected==1);
        p.paragraph(available?"Return here when you leave GameSwitcher.":"Automatic return is unavailable in this session.",y+24);
        p.footer({{"A",selected==0?"Toggle":"Details"},{"B","Back"}});
    } else if(page==MenuPage::Help) {
        pages_=2;page_=std::min(page_,pages_-1);
        const bool browser=page_==0;
        const std::vector<std::pair<std::string,std::string>> browserRows={
            {"UP DOWN","Move selection"},{"LEFT RIGHT","Change console"},{"A","Launch"},{"B","Exit"},
            {"SELECT","Actions"},{"Y","Settings"},{"MENU","GameSwitcher"}};
        const std::vector<std::pair<std::string,std::string>> menuRows={
            {"UP DOWN","Move selection"},{"A","Choose"},{"B","Back"},{"MENU","Close menu"}};
        text(screen,resources_.bodyFont,browser?"Favorites list":"Inside menus",list,margin,header+8);
        const int dividerY=header+8+lineHeight(resources_.bodyFont)+4;
        fill(screen,{0,dividerY,640,2},{Uint8(theme_.currentPageRed),Uint8(theme_.currentPageGreen),Uint8(theme_.currentPageBlue),255});
        if(divider_) {SDL_Rect src{0,0,std::min(640,divider_->w),std::min(2,divider_->h)},dest{0,dividerY,src.w,src.h};SDL_BlitSurface(divider_,&src,screen,&dest);}
        int y=dividerY+10;
        const auto& controls=browser?browserRows:menuRows;
        const int h=std::max(38,lineHeight(resources_.bodyFont)+8);
        for(const auto& item:controls){
            control(screen,controlLabel(item.first),margin,y+h/2);
            text(screen,resources_.bodyFont,item.second,list,210,y+(h-TTF_FontHeight(resources_.bodyFont))/2);y+=h;
        }
        p.footer({{browser?"DOWN":"UP",browser?"Inside menus":"Favorites list"},{"B","Back"}});
    } else if(page==MenuPage::ReturnInfo) {
        const std::vector<std::string> paragraphs={
            available?"Integration: available":"Integration: unavailable (optional runtime patch required)",
            "On", "B or START in GameSwitcher returns here. Switching games keeps the session. A resumes the game.",
            "Off", "Use Onion's ordinary menu return. Direct game exit ends the return session."};
        std::vector<std::string> lines;
        for(const auto& value:paragraphs){auto wrapped=wrap(resources_.bodyFont,value,600);lines.insert(lines.end(),wrapped.begin(),wrapped.end());lines.emplace_back();}
        const int h=lineHeight(resources_.bodyFont)+4, perPage=std::max(1,340/h);
        pages_=std::max(1,(int(lines.size())+perPage-1)/perPage);page_=std::min(page_,pages_-1);
        int y=header+12;for(int i=page_*perPage;i<std::min(int(lines.size()),(page_+1)*perPage);++i){text(screen,resources_.bodyFont,lines[i],list,margin,y);y+=h;}
        if(pages_>1)p.footer({{"UP DOWN","Page"},{"B","Back"}});else p.footer({{"B","Back"}});
    }

    if(pages_>1 && page!=MenuPage::RemoveConfirm && !theme_.hideHints)text(screen,resources_.hintFont,std::to_string(page_+1)+" of "+std::to_string(pages_),hint,620-width(resources_.hintFont,std::to_string(page_+1)+" of "+std::to_string(pages_)),450-TTF_FontHeight(resources_.hintFont)/2);
}

void MenuRenderer::drawError(SDL_Surface* screen,const std::string& message,Uint32 ticks) {
    (void)ticks;
    auto font=resources_.bodyFont;
    const auto ink=color(theme_.list);
    const auto lines=wrap(font,conciseMenuError(message),600);
    const int h=lineHeight(font), height=int(lines.size())*h+16;
    const auto bg=backgroundColor_;
    fill(screen,{0,bottom-height,640,height},popupColor(bg,ink));
    int y=bottom-height+8;
    for(const auto& line:lines){text(screen,font,line,ink,margin,y);y+=h;}
}

#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
void MenuRenderer::drawControlSamples(SDL_Surface* screen) {
    int x=20;
    for(const auto& key:{"A","B","X","Y","SELECT","START","MENU"})x+=control(screen,controlLabel(key),x,350)+12;
}
#endif
