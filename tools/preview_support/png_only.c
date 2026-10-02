// Host preview uses upstream SDL_image's PNG backend; other codecs are unnecessary.
#include <SDL_image.h>
extern int IMG_InitPNG(void);
extern void IMG_QuitPNG(void);
int IMG_Init(int flags){return (flags & IMG_INIT_PNG) && IMG_InitPNG()==0 ? IMG_INIT_PNG : 0;}
void IMG_Quit(void){IMG_QuitPNG();}
SDL_Surface* IMG_Load(const char* path){SDL_RWops* file=SDL_RWFromFile(path,"rb");if(!file)return NULL;SDL_Surface* image=IMG_LoadPNG_RW(file);SDL_RWclose(file);return image;}
