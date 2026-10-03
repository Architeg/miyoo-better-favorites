#include "profiled_sdl.h"
#include "menu_renderer.h"
#include "menu_text.h"
#include "browser_resources.h"
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <array>
#include <cstring>
#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <fstream>
#include <cctype>
#include <dirent.h>
#include <cstdlib>
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
// Check the already-composited surface, including theme dialog/background art.
// A theme section color is optional secondary ink; normal theme text is the fallback.
double relativeLight(SDL_Color c) {
    static const auto linear=[] {std::array<double,256> values{};
        for(int i=0;i<256;++i){const double v=i/255.0;values[i]=v<=0.04045?v/12.92:std::pow((v+0.055)/1.055,2.4);}return values;}();
    return 0.2126*linear[c.r]+0.7152*linear[c.g]+0.0722*linear[c.b];
}
SDL_Color secondaryInk(SDL_Surface* screen,SDL_Rect area,SDL_Color section,SDL_Color normal) {
    SDL_Rect clipped;if(!SDL_IntersectRect(&area,&screen->clip_rect,&clipped))return normal;
    if(SDL_LockSurface(screen)!=0)return normal;
    const double ink=relativeLight(section)+0.05;bool readable=true;
    for(int y=clipped.y;y<clipped.y+clipped.h && readable;y+=4)for(int x=clipped.x;x<clipped.x+clipped.w;x+=4){
        const int bytes=screen->format->BytesPerPixel;
        const auto* at=static_cast<const Uint8*>(screen->pixels)+y*screen->pitch+x*bytes;
        Uint32 pixel=0;
        if(bytes==3)pixel=SDL_BYTEORDER==SDL_BIG_ENDIAN?(at[0]<<16|at[1]<<8|at[2]):(at[0]|at[1]<<8|at[2]<<16);
        else if(bytes==2){Uint16 shortPixel;std::memcpy(&shortPixel,at,2);pixel=shortPixel;}
        else std::memcpy(&pixel,at,bytes);
        SDL_Color bg{};SDL_GetRGB(pixel,screen->format,&bg.r,&bg.g,&bg.b);
        const double surface=relativeLight(bg)+0.05;
        if(std::max(ink,surface)/std::min(ink,surface)<4.5){readable=false;break;}
    }
    SDL_UnlockSurface(screen);return readable?section:normal;
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
std::string canonical(const std::string& path) {
    char* resolved=realpath(path.c_str(),nullptr);if(!resolved)return {};
    std::string result=resolved;std::free(resolved);return result;
}
bool suppliedFont(const Theme& theme,const std::string& path) {
    const auto file=canonical(path);if(file.empty())return false;
    for(const auto& root:theme.regularFontRoots){const auto base=canonical(root);if(!base.empty()&&file.rfind(base+"/",0)==0)return true;}
    return false;
}
TTF_Font* readableFont(const Theme& theme,const ThemeTextStyle& style,int maxHeight=0) {
    std::string path=style.fontPath;
    const auto dot=path.find_last_of('.');
    std::string stem=dot==std::string::npos?path:path.substr(0,dot);
    for(const std::string suffix:{"-Regular","_Regular"}) {
        if(stem.size()>=suffix.size() && stem.compare(stem.size()-suffix.size(),suffix.size(),suffix)==0){stem.resize(stem.size()-suffix.size());break;}
    }
    // Only a heavier face in the same family/directory is eligible.
    for(const auto* suffix:{"-SemiBold.otf","-SemiBold.ttf","-Bold.otf","-Bold.ttf"}) {
        const auto candidate=stem+suffix;
        if(suppliedFont(theme,candidate)&&std::ifstream(candidate).good()){path=candidate;break;}
    }
    for(int size=style.size+1;size>=8;--size) {
        auto* font=profiledFontOpen(path.c_str(),size);if(!font)return nullptr;
        std::string face=TTF_FontFaceStyleName(font)?TTF_FontFaceStyleName(font):"";
        std::transform(face.begin(),face.end(),face.begin(),[](unsigned char c){return std::tolower(c);});
        if(face.find("bold")==std::string::npos)TTF_SetFontStyle(font,TTF_STYLE_BOLD);
        if(!maxHeight || TTF_FontHeight(font)<=maxHeight)return font;
        TTF_CloseFont(font);
    }
    return nullptr;
}
// Only an actual same-family regular face supplied by the current profile/theme is eligible.
// The inherited theme face remains the honest fallback when none is installed.
bool regularFace(TTF_Font* font) {
    if (!font || (TTF_GetFontStyle(font) & (TTF_STYLE_BOLD | TTF_STYLE_ITALIC))) return false;
    std::string style=TTF_FontFaceStyleName(font)?TTF_FontFaceStyleName(font):"";
    std::transform(style.begin(),style.end(),style.begin(),[](unsigned char c){return std::tolower(c);});
    return style.empty() || style=="regular" || style=="normal" || style=="book" || style=="roman";
}
TTF_Font* regularFont(const Theme& theme,const ThemeTextStyle& style,int size) {
    auto* original=profiledFontOpen(style.fontPath.c_str(),size);
    if(!original)return nullptr;
    const std::string family=TTF_FontFaceFamilyName(original)?TTF_FontFaceFamilyName(original):"";
    std::string chosen=style.fontPath;
    if(!regularFace(original) && !family.empty()) {
        // Search only supplied current-profile/active-theme faces. A configured
        // absolute font outside these roots is usable, but does not authorize a
        // scan of its siblings (which could belong to an unrelated theme).
        std::vector<std::string> roots;
        for(const auto& root:theme.regularFontRoots){const auto resolved=canonical(root);if(!resolved.empty())roots.push_back(resolved);}
        const auto resolvedFont=canonical(style.fontPath);
        std::string relativeDirectory;
        for(const auto& root:roots) {
            if(resolvedFont.rfind(root+"/",0)==0) {
                const auto relative=resolvedFont.substr(root.size()+1);
                const auto slash=relative.find_last_of('/');
                if(slash!=std::string::npos)relativeDirectory=relative.substr(0,slash);
                break;
            }
        }
        std::vector<std::string> paths;
        for(const auto& root:roots) {
            std::vector<std::string> directories{root};
            if(!relativeDirectory.empty())directories.insert(directories.begin(),root+"/"+relativeDirectory);
            std::vector<std::string> supplied;
            for(const auto& directory:directories) {
                const auto resolvedDirectory=canonical(directory);
                const bool allowed=std::any_of(roots.begin(),roots.end(),[&](const std::string& root){return resolvedDirectory==root || resolvedDirectory.rfind(root+"/",0)==0;});
                if(!allowed)continue;
                auto* entries=opendir(resolvedDirectory.c_str());if(!entries)continue;
                while(auto* entry=readdir(entries)) {
                    std::string name=entry->d_name;const auto dot=name.find_last_of('.');
                    if(dot!=std::string::npos && (name.substr(dot)==".ttf" || name.substr(dot)==".otf"))supplied.push_back(directory+"/"+name);
                }
                closedir(entries);
            }
            std::sort(supplied.begin(),supplied.end());
            paths.insert(paths.end(),supplied.begin(),supplied.end());
        }
        for(const auto& path:paths) {
            if(!suppliedFont(theme,path))continue;
            auto* candidate=profiledFontOpen(path.c_str(),size);if(!candidate)continue;
            if(regularFace(candidate) && TTF_FontFaceFamilyName(candidate) && family==TTF_FontFaceFamilyName(candidate)) {
                TTF_CloseFont(original);original=candidate;chosen=path;break;
            }
            TTF_CloseFont(candidate);
        }
    }
    std::cerr<<"Menu explanation font: "<<chosen<<" size="<<size<<" face="
        <<(TTF_FontFaceStyleName(original)?TTF_FontFaceStyleName(original):"unknown")
        <<(regularFace(original)?" (regular)":" (no same-family regular face available; native weight retained)")<<std::endl;
    return original;
}
// Inline words and cached badges share one measured text flow. Punctuation stays
// attached to its badge; wrapped continuation lines start at the same left edge.
struct FlowWord {std::string text,suffix;SDL_Surface* badge=nullptr;int x=0,line=0,w=0;};
struct TextFlow {std::vector<FlowWord> words;int lines=0,lineHeight=0;};
TextFlow inlineFlow(TTF_Font* font,const std::string& sentence,int maxWidth,
                    const std::function<SDL_Surface*(const std::string&)>& badge) {
    TextFlow flow;flow.lineHeight=std::max(lineHeight(font),TTF_FontHeight(font))+4;
    std::istringstream input(sentence);std::string token;
    int x=0,line=0;const int gap=width(font," ");
    while(input>>token) {
        FlowWord word;word.text=token;
        if(token.front()=='[') {
            const auto end=token.find(']');
            if(end!=std::string::npos){word.badge=badge(token.substr(1,end-1));word.suffix=token.substr(end+1);}
        }
        word.w=word.badge?word.badge->w+width(font,word.suffix):width(font,word.text);
        if(word.badge)flow.lineHeight=std::max(flow.lineHeight,word.badge->h+4);
        if(x && x+gap+word.w>maxWidth){++line;x=0;}
        if(x)x+=gap;
        word.x=x;word.line=line;flow.words.push_back(word);x+=word.w;
    }
    flow.lines=flow.words.empty()?0:line+1;return flow;
}
void drawFlow(SDL_Surface* screen,TTF_Font* font,const TextFlow& flow,int x,int y,SDL_Color ink) {
    for(const auto& word:flow.words) {
        const int center=y+word.line*flow.lineHeight+flow.lineHeight/2;
        if(word.badge){control(screen,word.badge,x+word.x,center);text(screen,font,word.suffix,ink,x+word.x+word.badge->w,center-TTF_FontHeight(font)/2);}
        else text(screen,font,word.text,ink,x+word.x,center-TTF_FontHeight(font)/2);
    }
}
void chevron(SDL_Surface* screen,SDL_Surface* asset,int x,int cy,bool right,SDL_Color ink) {
    if(asset){blit(asset,screen,{x,cy-12,24,24});return;}
    // Two diagonal strokes, without an arrow shaft.
    for(int i=0;i<=8;++i){const int px=x+7+(right?8-i:i);fill(screen,{px,cy-i,2,2},ink);fill(screen,{px,cy+i,2,2},ink);}
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
    SDL_Surface *leftChevron, *rightChevron;
    int row(const std::string& label,const std::string& value,int y,int w,bool active,bool disabled=false,bool adjustable=false) {
        auto font=resources.listFont;
        const int arrowWidth=adjustable?64:0;
        const int valueWidth=value.empty()?0:width(font,value)+24+arrowWidth;
        const auto lines=wrap(font,label,w-margin*2-valueWidth);
        const int height=std::max(rowHeight,int(lines.size())*lineHeight(font)+16);
        if(active) {
            if(resources.selection) blit(resources.selection,screen,{0,y+(height-resources.selection->h)/2,w,resources.selection->h});
            else fill(screen,{0,y,w,height},mix(panel,list,18));
        }
        SDL_Color c=active?selected:list; if(disabled)c=mix(c,panel,60);
        int top=y+(height-int(lines.size())*lineHeight(font))/2;
        for(const auto& line:lines) {text(screen,font,line,c,margin,top);top+=lineHeight(font);}
        if(!value.empty()) {
            int right=w-margin-(adjustable?32:0);
            if(adjustable&&active)chevron(screen,rightChevron,w-margin-24,y+height/2,true,c);
            const int valueX=right-width(font,value);
            text(screen,font,value,c,valueX,y+(height-TTF_FontHeight(font))/2);
            if(adjustable&&active)chevron(screen,leftChevron,valueX-32,y+height/2,false,c);
        }
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
MenuRenderer::MenuRenderer(const Theme& theme,MenuResources resources):sectionHeadingFont_(resources.bodyFont),theme_(theme),resources_(resources) {
    if(!theme.actionMenuPath.empty()) popup_=loadThemeImage(theme,theme.actionMenuPath);
    if(!theme.menuLeftArrowPath.empty()) leftArrow_=loadThemeImage(theme,theme.menuLeftArrowPath);
    if(!theme.menuRightArrowPath.empty()) rightArrow_=loadThemeImage(theme,theme.menuRightArrowPath);
    // App-owned instances only. Browser resources are borrowed and never restyled.
    returnHeadingFont_=profiledFontOpen(theme.section.fontPath.c_str(),theme.section.size);
    if(returnHeadingFont_)TTF_SetFontStyle(returnHeadingFont_,TTF_GetFontStyle(returnHeadingFont_)|TTF_STYLE_BOLD);
    ownedBodyFont_=readableFont(theme,theme.section);
    ownedHintFont_=readableFont(theme,theme.hint,26);
    if(ownedBodyFont_)resources_.bodyFont=ownedBodyFont_;
    if(ownedHintFont_)resources_.hintFont=ownedHintFont_;
    // Preserve the established body size for modal notes; regular weight is a
    // face choice, never a smaller substitute for the note's readable size.
    regularBodyFont_=regularFont(theme,theme.section,theme.section.size+1);
    // Increase the preceding fitted description size by two points. Wrapping
    // never shrinks it; face selection happens once at construction.
    int descriptionSize=std::max(16,theme.section.size-1);
    for(;descriptionSize>8;--descriptionSize){
        auto* probe=profiledFontOpen(theme.section.fontPath.c_str(),descriptionSize);
        if(!probe)break;
        const int h=TTF_FontHeight(probe);TTF_CloseFont(probe);if(h<=24)break;
    }
    descriptionFont_=regularFont(theme,theme.section,descriptionSize+2);
    const auto ink=color(theme_.list);
    backgroundColor_=sample(resources_.background,contrastBase(ink));
    panelColor_=popupColor(backgroundColor_,ink);
    if(!theme.dialogPath.empty()) {
        auto* asset=loadThemeImage(theme,theme.dialogPath);
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
    if(!theme.horizontalDividerPath.empty())divider_=loadThemeImage(theme,theme.horizontalDividerPath);
    shade_=SDL_CreateRGBSurfaceWithFormat(0,640,480,32,SDL_PIXELFORMAT_RGBA32);
    if(shade_){SDL_FillRect(shade_,nullptr,SDL_MapRGBA(shade_->format,backgroundColor_.r,backgroundColor_.g,backgroundColor_.b,170));SDL_SetSurfaceBlendMode(shade_,SDL_BLENDMODE_BLEND);}
}
SDL_Surface* MenuRenderer::controlLabel(const std::string& key) {
    auto found=controls_.find(key);if(found!=controls_.end())return found->second;
    auto* font=resources_.hintFont;const auto ink=color(theme_.hint);
    const int h=std::max(28,TTF_FontHeight(font)+6);
    const bool direction=key=="UP"||key=="DOWN"||key=="LEFT"||key=="RIGHT"||key=="UP DOWN"||key=="LEFT RIGHT";
    const bool face=key.size()==1 && std::string("ABXY").find(key)!=std::string::npos;
    const int w=key=="L1 R1"?width(font,"L1")+width(font,"R1")+40:direction?(key.find(' ')==std::string::npos?h:h*2+8):face?h:width(font,key)+16;
    auto* surface=SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_RGBA32);
    if(!surface)return nullptr;
    SDL_FillRect(surface,nullptr,SDL_MapRGBA(surface->format,0,0,0,0));
    SDL_SetSurfaceBlendMode(surface,SDL_BLENDMODE_BLEND);
    if(key=="L1 R1") {
        auto* left=controlLabel("L1");auto* right=controlLabel("R1");
        if(left && right){blit(left,surface,{0,0,left->w,h});blit(right,surface,{left->w+8,0,right->w,h});}
    } else if(direction) {
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
    if(returnHeadingFont_)TTF_CloseFont(returnHeadingFont_);
    returnHeadingFont_=nullptr;
    if(regularBodyFont_)TTF_CloseFont(regularBodyFont_);
    regularBodyFont_=nullptr;
    if(descriptionFont_)TTF_CloseFont(descriptionFont_);
    descriptionFont_=nullptr;
    if(ownedBodyFont_)TTF_CloseFont(ownedBodyFont_);
    if(ownedHintFont_)TTF_CloseFont(ownedHintFont_);
    ownedBodyFont_=ownedHintFont_=nullptr;
}
MenuRenderer::~MenuRenderer(){release();}
void MenuRenderer::movePage(int direction){page_=std::max(0,std::min(pages_-1,page_+direction));}
void MenuRenderer::draw(SDL_Surface* screen,MenuPage page,std::size_t selected,bool hasFavorite,
                        bool returnOn,bool available,const std::string& gameTitle,Uint32 ticks,const std::string& error,const AppSettings& settings) {
    (void)ticks;
    if(previous_!=page){page_=0;pages_=1;previous_=page;}
    const auto list=color(theme_.list),hint=color(theme_.hint);
    const SDL_Color selection{Uint8(theme_.selectedRed),Uint8(theme_.selectedGreen),Uint8(theme_.selectedBlue),255};
    const SDL_Color section{Uint8(theme_.currentPageRed),Uint8(theme_.currentPageGreen),Uint8(theme_.currentPageBlue),255};
    const auto bg=backgroundColor_;
    const auto surface=panelColor_;
    Painter p{screen,theme_,resources_,list,selection,hint,bg,surface,[&](const std::string& key){return controlLabel(key);},theme_.hideIcons?nullptr:leftArrow_,theme_.hideIcons?nullptr:rightArrow_};
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
        constexpr int inset=24,panelWidth=520,inner=panelWidth-inset*2;
        const auto body=regularBodyFont_?regularBodyFont_:resources_.bodyFont;
        const int titleH=lineHeight(resources_.titleFont),textH=lineHeight(body),lineH=lineHeight(resources_.listFont);
        const int choiceH=std::max(40,TTF_FontHeight(resources_.listFont)+12);
        const int pagingH=std::max(36,TTF_FontHeight(resources_.hintFont)+12);
        const int footerH=theme_.hideHints?0:std::max(44,TTF_FontHeight(resources_.hintFont)+16);
        const auto heading=wrap(resources_.titleFont,"Remove from Favorites?",inner);
        const auto lines=wrap(resources_.listFont,gameTitle.empty()?"No game selected":gameTitle,inner);
        const auto note=wrap(body,"The game file will be kept.",inner);
        const auto errors=error.empty()?std::vector<std::string>{}:wrap(body,conciseMenuError(error),inner);
        const int base=48+int(heading.size())*titleH+12+12+int(note.size())*textH+
            (errors.empty()?0:12+int(errors.size())*textH)+16+choiceH+(footerH?12+footerH:0);
        int perPage=std::max(1,std::min(3,(432-base)/lineH));
        pages_=std::max(1,(int(lines.size())+perPage-1)/perPage);
        if(pages_>1){perPage=std::max(1,std::min(3,(432-base-pagingH)/lineH));pages_=std::max(1,(int(lines.size())+perPage-1)/perPage);}
        page_=std::min(page_,pages_-1);
        const int visible=std::min(perPage,int(lines.size())-page_*perPage);
        const int panelHeight=base+visible*lineH+(pages_>1?pagingH:0);
        const int x=(640-panelWidth)/2,top=(480-panelHeight)/2;
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
        assert(panelHeight<=432 && top>=24);
#endif
        SDL_Rect panel{x,top,panelWidth,panelHeight};fill(screen,panel,surface);
        if(dialog_)blit(dialog_,screen,panel);
        boundary(screen,panel,mix(surface,list,22));
        int y=top+inset;
        for(const auto& line:heading){text(screen,resources_.titleFont,line,color(theme_.title),x+(panelWidth-width(resources_.titleFont,line))/2,y);y+=titleH;}
        y+=12;
        for(int i=page_*perPage;i<page_*perPage+visible;++i){text(screen,resources_.listFont,lines[i],list,x+(panelWidth-width(resources_.listFont,lines[i]))/2,y);y+=lineH;}
        if(pages_>1){
            const int cy=y+pagingH/2;
            text(screen,resources_.hintFont,std::to_string(page_+1)+" of "+std::to_string(pages_),hint,x+inset,cy-TTF_FontHeight(resources_.hintFont)/2);
            auto* badge=controlLabel("UP DOWN");
            const int labelWidth=width(resources_.hintFont,"Title pages");
            control(screen,badge,x+panelWidth-inset-labelWidth-10-(badge?badge->w:0),cy);
            text(screen,resources_.hintFont,"Title pages",hint,x+panelWidth-inset-labelWidth,cy-TTF_FontHeight(resources_.hintFont)/2);
            y+=pagingH;
        }
        y+=12;
        const auto noteInk=secondaryInk(screen,{x+inset,y,inner,int(note.size())*textH},section,list);
        for(const auto& line:note){text(screen,body,line,noteInk,x+(panelWidth-width(body,line))/2,y);y+=textH;}
        if(!errors.empty())y+=12;
        for(const auto& line:errors){text(screen,body,line,list,x+inset,y);y+=textH;}
        y+=16;
        const int choiceW=(inner-16)/2;
        for(std::size_t i=0;i<2;++i){
            SDL_Rect choice{x+inset+int(i)*(choiceW+16),y,choiceW,choiceH};
            if(selected==i){
                if(resources_.selection)blit(resources_.selection,screen,choice);
                else fill(screen,choice,mix(surface,list,18));
            }
            centered(screen,resources_.listFont,i==0?"Cancel":"Remove",selected==i?selection:list,choice);
        }
        y+=choiceH;
        if(footerH){y+=12;p.hints({{"A","Choose"},{"B","Back"}},x+inset,y+footerH/2);y+=footerH;}
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
        assert(y==top+panelHeight-inset);
#endif
        return;
    }
    fill(screen,{0,0,640,480},bg);blit(resources_.background,screen,{0,0,640,480});blit(resources_.title,screen,{0,0,640,header});
    const std::string title=page==MenuPage::Settings?"SETTINGS":page==MenuPage::Help?"HELP":
        page==MenuPage::ReturnInfo?"AUTOMATIC RETURN":page==MenuPage::HomeInfo?"HOME FAVORITES":"REMOVE FROM FAVORITES?";
    const auto titleLines=wrap(resources_.titleFont,title,600);
    // Titles are short fixed labels, measured and wrapped rather than clipped.
    int headingY=(header-int(titleLines.size())*lineHeight(resources_.titleFont))/2;
    for(const auto& line:titleLines){text(screen,resources_.titleFont,line,color(theme_.title),(640-width(resources_.titleFont,line))/2,headingY);headingY+=lineHeight(resources_.titleFont);}
    if(page==MenuPage::Settings) {
        const char* labels[]={"Automatic return","Group by console","Numeric prefixes","Sorting","Replace stock Favorites","About automatic return","About Home Favorites"};
        const std::string values[]={returnOn?"ON":"OFF",settings.groupByConsole?"ON":"OFF",settings.showNumericPrefixes?"Show":"Hide",settings.sortMode==SortMode::OriginalLabel?"Original label":"Alphabetical title",settings.replaceStockFavorites?"ON":"OFF","",""};
        const auto font=descriptionFont_?descriptionFont_:resources_.bodyFont;
        // Reserve two lines at the existing font/badge size, regardless of row count
        // or selected description. The surface touches the footer at y=420.
        const auto twoLineMetrics=inlineFlow(font,"[B] [START]",600,[&](const std::string& key){return controlLabel(key);});
        const int panelHeight=2*twoLineMetrics.lineHeight+16;
        const SDL_Rect descriptionPanel{0,bottom-panelHeight,640,panelHeight};
        const int rowsBottom=descriptionPanel.y-8;
        fill(screen,descriptionPanel,surface);
        // No selection asset: this is a quiet, opaque theme-derived popup surface.
        // Only fully measured rows fit above the independently anchored panel.
        auto settingHeight=[&](int i){
            const int arrows=i<5?64:0;
            const int valueWidth=values[i].empty()?0:width(resources_.listFont,values[i])+24+arrows;
            return std::max(60,int(wrap(resources_.listFont,labels[i],600-valueWidth).size())*lineHeight(resources_.listFont)+16);
        };
        int first=int(selected), used=settingHeight(first);
        while(first>0 && used+settingHeight(first-1)<=rowsBottom-header){--first;used+=settingHeight(first);}
        int y=header;
        for(int i=first;i<7 && y+settingHeight(i)<=rowsBottom;++i)y+=p.row(labels[i],values[i],y,640,selected==std::size_t(i),false,i<5);
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
        assert(y<=rowsBottom && descriptionPanel.y+descriptionPanel.h==bottom);
#endif
        const std::string descriptions[]={returnOn?"[B] / [START]: return here from GameSwitcher.":"",
            settings.groupByConsole?"Group games under console headings.":"Flat list. Console jumps are disabled.",
            settings.showNumericPrefixes?"Show numeric prefixes in displayed titles.":"Hide leading numeric prefixes. Sorting is unchanged.",
            settings.sortMode==SortMode::OriginalLabel?"Sort by literal stored labels.":"Sort titles without leading numeric prefixes.",
            settings.homeIntegrationAvailable?"Home integration installed.":"Integration unavailable. Stock Favorites remains.",
            "Read how automatic return works.","Installation and Home return behavior."};
        const auto description=inlineFlow(font,descriptions[selected],600,[&](const std::string& key){return controlLabel(key);});
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
        assert(description.lines<=2 && description.lines*description.lineHeight+16<=panelHeight);
#endif
        const int descriptionHeight=description.lines*description.lineHeight;
        const int descriptionY=descriptionPanel.y+(descriptionPanel.h-descriptionHeight)/2;
        const auto descriptionInk=secondaryInk(screen,{margin,descriptionY,600,descriptionHeight},section,list);
        drawFlow(screen,font,description,margin,descriptionY,descriptionInk);
        p.footer({{"A",selected<5?"Change":"Open"},{"B","Back"}});
    } else if(page==MenuPage::Help) {
        pages_=2;page_=std::min(page_,pages_-1);
        const bool browser=page_==0;
        const std::vector<std::pair<std::string,std::string>> browserRows={
            {"UP DOWN","Move selection"},{"L1 R1","Page up / down"},{"LEFT RIGHT",settings.groupByConsole?"Change console":"Console jumps disabled"},{"A","Launch"},{"B","Exit"},
            {"SELECT","Actions"},{"Y","Settings"},{"MENU","GameSwitcher"}};
        const std::vector<std::pair<std::string,std::string>> menuRows={
            {"UP DOWN","Move selection"},{"A","Choose"},{"B","Back"},{"LEFT RIGHT","Change value"},{"MENU","Close menu"}};
        text(screen,resources_.bodyFont,browser?"Favorites list":"Inside menus",list,margin,header+8);
        const int dividerY=header+8+lineHeight(resources_.bodyFont)+4;
        fill(screen,{0,dividerY,640,2},{Uint8(theme_.currentPageRed),Uint8(theme_.currentPageGreen),Uint8(theme_.currentPageBlue),255});
        if(divider_) {SDL_Rect src{0,0,std::min(640,divider_->w),std::min(2,divider_->h)},dest{0,dividerY,src.w,src.h};SDL_BlitSurface(divider_,&src,screen,&dest);}
        int y=dividerY+10;
        const auto& controls=browser?browserRows:menuRows;
        const int h=std::max(38,lineHeight(resources_.bodyFont)+8);
#ifdef BETTER_FAVORITES_MENU_RENDER_TESTING
        assert(y+int(controls.size())*h<=bottom);
        assert(controlLabel("L1 R1")->w<190);
#endif
        for(const auto& item:controls){
            control(screen,controlLabel(item.first),margin,y+h/2);
            text(screen,resources_.bodyFont,item.second,list,210,y+(h-TTF_FontHeight(resources_.bodyFont))/2);y+=h;
        }
        p.footer({{browser?"DOWN":"UP",browser?"Inside menus":"Favorites list"},{"B","Back"}});
    } else if(page==MenuPage::ReturnInfo || page==MenuPage::HomeInfo) {
        const auto font=descriptionFont_?descriptionFont_:resources_.bodyFont;
        struct Block {std::string value;bool heading;};
        const std::vector<Block> blocks=page==MenuPage::HomeInfo?std::vector<Block>{
            {settings.homeIntegrationAvailable?"Integration: installed":"Integration: unavailable",false},
            {"Replace stock Favorites",true},{"ON opens Better Favorites from Home. OFF keeps stock Favorites.",false},
            {"[B]: exit to Onion. Home restoration is under test.",false},{"Apps access is unchanged. Automatic return is independent.",false},
            {"Install or remove only on a powered-off card. Binary changes require reboot.",false}}:std::vector<Block>{
            {available?"Integration: available":"Integration: unavailable (optional patch required)",false},
            {"When enabled",true},{"[B] / [START]: return here from GameSwitcher.",false},
            {"[A]: resume the game. Switching games keeps the session.",false},
            {"When disabled",true},{"Use Onion's ordinary menu return.",false},
            {"Direct game exit ends the return session.",false}};
        struct Positioned {TextFlow flow;TTF_Font* font;int y,page;bool heading;};
        std::vector<Positioned> laidOut;int y=header+8,pageIndex=0;
        for(std::size_t i=0;i<blocks.size();++i){
            const auto& block=blocks[i];auto* face=block.heading?(returnHeadingFont_?returnHeadingFont_:sectionHeadingFont_):font;
            if(block.heading)y+=12;
            auto flow=inlineFlow(face,block.value,600,[&](const std::string& key){return controlLabel(key);});
            const int height=flow.lines*flow.lineHeight;
            int keep=height;
            if(block.heading && i+1<blocks.size())keep+=4+inlineFlow(font,blocks[i+1].value,600,[&](const std::string& key){return controlLabel(key);}).lineHeight;
            if(y+keep>bottom-4){++pageIndex;y=header+8;}
            laidOut.push_back({flow,face,y,pageIndex,block.heading});y+=height+(block.heading?2:8);
        }
        pages_=pageIndex+1;page_=std::min(page_,pages_-1);
        for(const auto& block:laidOut)if(block.page==page_){
            const auto ink=block.heading?SDL_Color{255,255,255,255}:secondaryInk(screen,{margin,block.y,600,block.flow.lines*block.flow.lineHeight},section,list);
            drawFlow(screen,block.font,block.flow,margin,block.y,ink);
        }
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
void testMenuSecondaryContrast() {
    for(const auto format:{SDL_PIXELFORMAT_RGB565,SDL_PIXELFORMAT_RGBA32}){
        auto* screen=SDL_CreateRGBSurfaceWithFormat(0,32,32,SDL_BITSPERPIXEL(format),format);assert(screen);
        const SDL_Color section{128,128,128,255},normal{255,255,255,255};
        fill(screen,{0,0,32,32},{0,0,0,255});
        auto ink=secondaryInk(screen,{0,0,32,32},section,normal);assert(ink.r==section.r);
        // A composited theme-art patch can invalidate an otherwise readable color.
        fill(screen,{8,8,16,16},section);
        ink=secondaryInk(screen,{0,0,32,32},section,normal);assert(ink.r==normal.r);
        SDL_FreeSurface(screen);
    }
}
bool MenuRenderer::explanationsAreRegular() const {return regularFace(regularBodyFont_) && regularFace(descriptionFont_);}
void MenuRenderer::verifyFonts(bool expectRegular) const {
    assert(regularBodyFont_ && descriptionFont_ && returnHeadingFont_);
    assert(TTF_GetFontStyle(returnHeadingFont_)&TTF_STYLE_BOLD);
    auto* heading=profiledFontOpen(theme_.section.fontPath.c_str(),theme_.section.size);assert(heading);
    assert(TTF_FontHeight(returnHeadingFont_)==TTF_FontHeight(heading));
    TTF_CloseFont(heading);
    if(expectRegular)assert(regularFace(regularBodyFont_) && regularFace(descriptionFont_));
    auto* original=profiledFontOpen(theme_.section.fontPath.c_str(),theme_.section.size+1);assert(original);
    assert(TTF_FontHeight(regularBodyFont_)>=TTF_FontHeight(original)-2);
    assert(std::string(TTF_FontFaceFamilyName(original))==TTF_FontFaceFamilyName(regularBodyFont_));
    if(regularFace(original))assert(regularFace(regularBodyFont_));
    TTF_CloseFont(original);
}
void MenuRenderer::drawControlSamples(SDL_Surface* screen) {
    int x=20;
    for(const auto& key:{"A","B","X","Y","SELECT","START","MENU"})x+=control(screen,controlLabel(key),x,350)+12;
}
#endif
