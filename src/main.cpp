#include "favorites_parser.h"
#include "navigation.h"
#include "theme_loader.h"
#include "ui_row.h"
#include "ui_rows.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <json.h>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace
{

bool drawTextCenteredVertically(
    SDL_Surface* destination,
    TTF_Font* font,
    const std::string& text,
    const SDL_Color& color,
    int x,
    int rowY,
    int rowHeight
)
{
    SDL_Surface* rendered =
        TTF_RenderUTF8_Blended(
            font,
            text.c_str(),
            color
        );

    if (!rendered) {
        return false;
    }

    const int y =
        rowY +
        (rowHeight - rendered->h) / 2;

    SDL_Rect target {
        x,
        y,
        rendered->w,
        rendered->h
    };

    const int result =
        SDL_BlitSurface(
            rendered,
            nullptr,
            destination,
            &target
        );

    SDL_FreeSurface(rendered);

    return result == 0;
}


std::string resolveNavigationSound(
    const Theme& theme
)
{
    /*
     * Match Onion's resource_getSoundChange():
     *
     * 1. active theme sound/change.wav
     * 2. Miyoo/Onion fallback sound/change.wav
     */
    const std::string themedPath =
        theme.rootPath +
        "/sound/change.wav";

    std::ifstream themedFile(
        themedPath,
        std::ios::binary
    );

    if (themedFile.good()) {
        return themedPath;
    }

    return
        "/mnt/SDCARD/miyoo/app/sound/change.wav";
}

int loadNavigationVolume()
{
    /*
     * Onion's settings loader reads bgmvol from the live
     * /mnt/SDCARD/system.json used by MainUI.
     */
    std::ifstream input(
        "/mnt/SDCARD/system.json"
    );

    if (!input.is_open()) {
        return 20;
    }

    std::string json(
        (
            std::istreambuf_iterator<char>(
                input
            )
        ),
        std::istreambuf_iterator<char>()
    );

    json_object* root =
        json_tokener_parse(
            json.c_str()
        );

    if (!root) {
        return 20;
    }

    int volume = 20;

    json_object* value = nullptr;

    if (
        json_object_object_get_ex(
            root,
            "bgmvol",
            &value
        ) &&
        value
    ) {
        volume =
            json_object_get_int(value);
    }

    json_object_put(root);

    return std::max(
        0,
        std::min(
            20,
            volume
        )
    );
}


void blitScaled(
    SDL_Surface* source,
    SDL_Surface* destination,
    const SDL_Rect& rectangle
)
{
    if (!source) {
        return;
    }

    SDL_Rect target = rectangle;

    SDL_BlitScaled(
        source,
        nullptr,
        destination,
        &target
    );
}

}

int main()
{
    constexpr int width = 640;
    constexpr int height = 480;
    constexpr int bpp = 16;

    ThemeLoader themeLoader("/mnt/SDCARD");
    const Theme theme = themeLoader.load();

    FavoritesParser parser("/mnt/SDCARD");

    const auto favorites =
        parser.loadFavorites(
            "/mnt/SDCARD/Roms/favourite.json"
        );

    const auto groups =
        parser.groupFavorites(favorites);

    const auto rows =
        buildUiRows(groups);

    std::size_t selectedRow =
        firstSelectableRow(rows);

    if (
        theme.rootPath.empty() ||
        selectedRow >= rows.size()
    ) {
        std::cerr
            << "Theme or favorites unavailable."
            << std::endl;

        return 1;
    }

    if (
        SDL_Init(
            SDL_INIT_VIDEO |
            SDL_INIT_AUDIO |
            SDL_INIT_EVENTS
        ) != 0
    ) {
        std::cerr
            << "SDL_Init failed: "
            << SDL_GetError()
            << std::endl;

        return 1;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        std::cerr
            << "IMG_Init failed: "
            << IMG_GetError()
            << std::endl;

        SDL_Quit();
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr
            << "TTF_Init failed: "
            << TTF_GetError()
            << std::endl;

        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window =
        SDL_CreateWindow(
            "Better Favorites",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            width,
            height,
            SDL_WINDOW_SHOWN
        );

    SDL_Renderer* renderer =
        SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED |
            SDL_RENDERER_PRESENTVSYNC
        );

    SDL_Surface* screen =
        SDL_CreateRGBSurface(
            0,
            width,
            height,
            bpp,
            0,
            0,
            0,
            0
        );

    SDL_Texture* texture =
        SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGB565,
            SDL_TEXTUREACCESS_STREAMING,
            width,
            height
        );

    if (
        !window ||
        !renderer ||
        !screen ||
        !texture
    ) {
        std::cerr
            << "SDL display setup failed: "
            << SDL_GetError()
            << std::endl;

        return 1;
    }

    SDL_Surface* background =
        IMG_Load(
            theme.backgroundPath.c_str()
        );

    SDL_Surface* titleBackground =
        IMG_Load(
            theme.titleBackgroundPath.c_str()
        );

    SDL_Surface* footerBackground = nullptr;

    if (!theme.footerBackgroundPath.empty()) {
        footerBackground =
            IMG_Load(
                theme.footerBackgroundPath.c_str()
            );
    }

    SDL_Surface* previewBackground = nullptr;

    if (!theme.previewBackgroundPath.empty()) {
        previewBackground =
            IMG_Load(
                theme.previewBackgroundPath.c_str()
            );

        if (!previewBackground) {
            std::cerr
                << "Preview background failed: "
                << theme.previewBackgroundPath
                << " : "
                << IMG_GetError()
                << std::endl;
        }
    }

    std::cerr
        << "THEME ICONS: hideIcons="
        << (theme.hideIcons ? "true" : "false")
        << " A="
        << theme.buttonAPath
        << " B="
        << theme.buttonBPath
        << std::endl;

    SDL_Surface* buttonA = nullptr;
    SDL_Surface* buttonB = nullptr;

    if (!theme.buttonAPath.empty()) {
        buttonA =
            IMG_Load(
                theme.buttonAPath.c_str()
            );

        if (!buttonA) {
            std::cerr
                << "A icon failed: "
                << theme.buttonAPath
                << " : "
                << IMG_GetError()
                << std::endl;
        }
    }

    if (!theme.buttonBPath.empty()) {
        buttonB =
            IMG_Load(
                theme.buttonBPath.c_str()
            );

        if (!buttonB) {
            std::cerr
                << "B icon failed: "
                << theme.buttonBPath
                << " : "
                << IMG_GetError()
                << std::endl;
        }
    }

    SDL_Surface* listSmall = nullptr;

    if (!theme.listSmallPath.empty()) {
        listSmall =
            IMG_Load(
                theme.listSmallPath.c_str()
            );
    }

    SDL_Surface* selectedPreview = nullptr;
    std::string selectedPreviewPath;

    /*
     * Onion-style navigation sound.
     *
     * The audio device and WAV are opened once. Navigation only calls
     * Mix_PlayChannel(), so there is no filesystem work while scrolling.
     */
    Mix_Chunk* navigationSound = nullptr;

    if (
        Mix_OpenAudio(
            48000,
            AUDIO_S16SYS,
            2,
            1024
        ) == 0
    ) {
        std::cerr
            << "Audio opened successfully."
            << std::endl;

        const std::string navigationSoundPath =
            resolveNavigationSound(theme);

        navigationSound =
            Mix_LoadWAV_RW(
                SDL_RWFromFile(
                    navigationSoundPath.c_str(),
                    "rb"
                ),
                1
            );

            std::cerr
                << "Navigation sound path: "
                << navigationSoundPath
                << std::endl;

            if (navigationSound) {
                std::cerr
                    << "Navigation sound loaded successfully."
                    << std::endl;
            }

        if (navigationSound) {
            const int navigationVolume =
                loadNavigationVolume();

            Mix_Volume(
                -1,
                (
                    navigationVolume *
                    MIX_MAX_VOLUME
                ) / 20
            );
        }
        else {
            std::cerr
                << "Navigation sound failed: "
                << navigationSoundPath
                << " : "
                << Mix_GetError()
                << std::endl;
        }
    }
    else {
        std::cerr
            << "Audio initialization failed: "
            << Mix_GetError()
            << std::endl;
    }

    SDL_Surface* divider = nullptr;

    if (!theme.horizontalDividerPath.empty()) {
        divider =
            IMG_Load(
                theme.horizontalDividerPath.c_str()
            );
    }

    TTF_Font* titleFont =
        TTF_OpenFont(
            theme.title.fontPath.c_str(),
            std::max(
                1,
                theme.title.size
            )
        );

    TTF_Font* listFont =
        TTF_OpenFont(
            theme.list.fontPath.c_str(),
            std::max(
                1,
                theme.list.size
            )
        );

    if (listFont) {
        TTF_SetFontStyle(
            listFont,
            TTF_STYLE_BOLD
        );
    }

    TTF_Font* sectionFont =
        TTF_OpenFont(
            theme.section.fontPath.c_str(),
            std::max(
                1,
                theme.section.size
            )
        );

    TTF_Font* footerFont =
        TTF_OpenFont(
            theme.hint.fontPath.c_str(),
            std::max(
                1,
                theme.hint.size
            )
        );

    if (
        !background ||
        !titleFont ||
        !listFont ||
        !sectionFont
    ) {
        std::cerr
            << "Required theme resources unavailable."
            << std::endl;

        return 1;
    }

    const SDL_Color titleColor {
        static_cast<Uint8>(
            theme.title.red
        ),
        static_cast<Uint8>(
            theme.title.green
        ),
        static_cast<Uint8>(
            theme.title.blue
        ),
        255
    };

    const SDL_Color listColor {
        static_cast<Uint8>(
            theme.list.red
        ),
        static_cast<Uint8>(
            theme.list.green
        ),
        static_cast<Uint8>(
            theme.list.blue
        ),
        255
    };

    const SDL_Color selectedTextColor {
        static_cast<Uint8>(
            theme.selectedRed
        ),
        static_cast<Uint8>(
            theme.selectedGreen
        ),
        static_cast<Uint8>(
            theme.selectedBlue
        ),
        255
    };

    const SDL_Color sectionColor {
        static_cast<Uint8>(
            theme.currentPageRed
        ),
        static_cast<Uint8>(
            theme.currentPageGreen
        ),
        static_cast<Uint8>(
            theme.currentPageBlue
        ),
        255
    };

    bool running = true;

    long firstRow = 0;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type != SDL_KEYDOWN) {
                continue;
            }

            const std::size_t previousSelection =
                selectedRow;

            bool navigationKey = false;

            switch (event.key.keysym.sym) {
            case SDLK_UP:
                navigationKey = true;
                selectedRow =
                    previousSelectableRow(
                        rows,
                        selectedRow
                    );
                break;

              case SDLK_DOWN:
                  navigationKey = true;

                  selectedRow =
                      nextSelectableRow(
                          rows,
                          selectedRow
                      );
                  break;

              case SDLK_LEFT:
                  navigationKey = true;

                  selectedRow =
                      previousConsoleRow(
                          rows,
                          selectedRow
                      );

                  if (
                      selectedRow <
                      rows.size()
                  ) {
                      const std::size_t headerRow =
                          consoleHeaderRow(
                              rows,
                              selectedRow
                          );

                      if (
                          headerRow <
                          rows.size()
                      ) {
                          firstRow =
                              static_cast<long>(
                                  headerRow
                              );
                      }
                  }

                  break;

              case SDLK_RIGHT:
                  navigationKey = true;

                  selectedRow =
                      nextConsoleRow(
                          rows,
                          selectedRow
                      );

                  if (
                      selectedRow <
                      rows.size()
                  ) {
                      const std::size_t headerRow =
                          consoleHeaderRow(
                              rows,
                              selectedRow
                          );

                      if (
                          headerRow <
                          rows.size()
                      ) {
                          firstRow =
                              static_cast<long>(
                                  headerRow
                              );
                      }
                  }

                  break;

              /*
               * Onion-style face-button feedback.
               *
               * Miyoo SDL mapping:
               *   A = Space
               *   B = Left Ctrl
               *   X = Left Shift
               *   Y = Left Alt
               *
               * Onion uses the same change.wav feedback for menu
               * actions as it does for navigation.
               */
              case SDLK_SPACE:
              case SDLK_LSHIFT:
              case SDLK_LALT:
                  if (
                      navigationSound &&
                      event.key.repeat == 0
                  ) {
                      Mix_PlayChannelTimed(
                          -1,
                          navigationSound,
                          0,
                          -1
                      );
                  }
                  break;

              case SDLK_LCTRL:
                  if (
                      navigationSound &&
                      event.key.repeat == 0
                  ) {
                      Mix_PlayChannelTimed(
                          -1,
                          navigationSound,
                          0,
                          -1
                      );

                      /*
                       * B exits immediately, so give the short UI
                       * sound enough time to reach the audio server
                       * before Mix_CloseAudio() runs.
                       */
                      SDL_Delay(50);
                  }

                  running = false;
                  break;

              case SDLK_ESCAPE:
                  running = false;
                  break;

            default:
                break;
            }

            /*
             * Match Onion's change-sound behavior: play only when
             * navigation actually changed the current selection.
             *
             * Reaching the beginning/end of the list therefore does
             * not produce a false navigation click.
             */
            if (
                navigationKey &&
                selectedRow != previousSelection &&
                navigationSound
            ) {

              std::cerr
                  << "Playing navigation sound."
                  << std::endl;

                Mix_PlayChannelTimed(
                    -1,
                    navigationSound,
                    0,
                    -1
                );
            }
          }

          /*
           * Load artwork for the currently selected Favorite.
           *
           * favourite.json is the source of truth. The parser already
           * provides the exact Onion imgpath through Favorite::imagePath.
           *
           * Cache the loaded surface so disk I/O happens only when the
           * selected game changes.
           */
          std::string newPreviewPath;

          if (
              selectedRow <
                  rows.size() &&
              rows[selectedRow].type ==
                  UiRowType::Favorite &&
              rows[selectedRow].favorite
          ) {
              newPreviewPath =
                  rows[selectedRow].favorite->imagePath;
          }

          if (
              newPreviewPath !=
              selectedPreviewPath
          ) {
              if (selectedPreview) {
                  SDL_FreeSurface(
                      selectedPreview
                  );

                  selectedPreview =
                      nullptr;
              }

              selectedPreviewPath =
                  newPreviewPath;

              if (!selectedPreviewPath.empty()) {
                  selectedPreview =
                      IMG_Load(
                          selectedPreviewPath.c_str()
                      );

                  if (!selectedPreview) {
                      std::cerr
                          << "Preview image failed: "
                          << selectedPreviewPath
                          << " : "
                          << IMG_GetError()
                          << std::endl;
                  }
              }
          }

          SDL_Rect fullScreen {
            0,
            0,
            width,
            height
        };

        blitScaled(
            background,
            screen,
            fullScreen
        );

        if (titleBackground) {
            SDL_Rect titleRect {
                0,
                0,
                640,
                60
            };

            blitScaled(
                titleBackground,
                screen,
                titleRect
            );
        }

        SDL_Surface* favoritesTitle =
            TTF_RenderUTF8_Blended(
                titleFont,
                "Favorites",
                titleColor
            );

        if (favoritesTitle) {
            SDL_Rect titleRect {
                (width - favoritesTitle->w) / 2,
                29 - favoritesTitle->h / 2,
                favoritesTitle->w,
                favoritesTitle->h
            };

            SDL_BlitSurface(
                favoritesTitle,
                nullptr,
                screen,
                &titleRect
            );

            SDL_FreeSurface(
                favoritesTitle
            );
        }

        /*
         * MainUI list geometry.
         *
         * The physical screen coordinates are fixed by Onion:
         *
         *   header:  0..59
         *   list:   60..419
         *   footer: 420..479
         *
         * Normal game rows are 60px.
         *
         * Console section headers are our own addition. Their typography
         * comes from the active theme; only the additional spacing is
         * Better Favorites-specific.
         */
        constexpr int contentTop = 60;
        constexpr int contentBottom = 420;

        constexpr int gameRowHeight = 60;

        constexpr int sectionTopGap = 8;
        constexpr int sectionTitleHeight = 40;
        constexpr int fallbackDividerHeight = 2;

        /*
         * Determine the visual height of a logical row.
         *
         * The divider belongs to the console heading, so the complete
         * section occupies:
         *
         *   spacing + title + divider
         */
        auto rowHeight =
            [&](const UiRow& row) -> int {
                if (
                    row.type ==
                    UiRowType::SystemDivider
                ) {
                    const int dividerHeight =
                        divider
                            ? std::min(
                                fallbackDividerHeight,
                                divider->h
                            )
                            : fallbackDividerHeight;

                    return
                        sectionTopGap +
                        sectionTitleHeight +
                        dividerHeight;
                }

                return gameRowHeight;
            };

         /*
          * Keep the scroll position stable while the selection moves.
          *
          * The selection is allowed to travel through the complete
          * visible area. The list only scrolls when the selected row
          * reaches an edge.
          *
          * Console headers have their real pixel height, so the same
          * logic works across mixed header/game rows.
          */

         if (
             selectedRow <
             static_cast<std::size_t>(
                 firstRow
             )
         ) {
             /*
              * Selection moved above the visible area.
              *
              * Move the viewport just enough to reveal it.
              */
             firstRow =
                 static_cast<long>(
                     selectedRow
                 );
         }

         /*
          * If the selected row is below the viewport, advance the
          * viewport until the complete selected row fits.
          */
         while (
             firstRow <
             static_cast<long>(
                 selectedRow
             )
         ) {
             int totalHeight = 0;

             for (
                 long i = firstRow;
                 i <=
                     static_cast<long>(
                         selectedRow
                     );
                 ++i
             ) {
                 totalHeight +=
                     rowHeight(
                         rows[
                             static_cast<std::size_t>(
                                 i
                             )
                         ]
                     );
             }

             /*
              * A sticky console header occupies real viewport space.
              *
              * When the viewport starts inside a console group, the games
              * cannot use the pixels occupied by that header.
              */
             int stickyReservedHeight = 0;

             if (
                 firstRow >= 0 &&
                 firstRow <
                     static_cast<long>(
                         rows.size()
                     ) &&
                 rows[
                     static_cast<std::size_t>(
                         firstRow
                     )
                 ].type ==
                     UiRowType::Favorite
             ) {
                 const std::size_t headerRow =
                     consoleHeaderRow(
                         rows,
                         static_cast<std::size_t>(
                             firstRow
                         )
                     );

                 if (
                     headerRow <
                     rows.size()
                 ) {
                     const int dividerHeight =
                         divider
                             ? std::min(
                                 fallbackDividerHeight,
                                 divider->h
                             )
                             : fallbackDividerHeight;

                     stickyReservedHeight =
                         sectionTopGap +
                         sectionTitleHeight +
                         dividerHeight;
                 }
             }

             const int availableHeight =
                 contentBottom -
                 contentTop -
                 stickyReservedHeight;

             if (
                 totalHeight <=
                 availableHeight
             ) {
                 break;
             }

             ++firstRow;
         }

         /*
          * Keep firstRow valid if the list is shorter than the viewport.
          */
         if (
             firstRow >
             static_cast<long>(
                 selectedRow
             )
         ) {
             firstRow =
                 static_cast<long>(
                     selectedRow
                 );
         }

        /*
         * Render forward until the physical list area is full.
         *
         * There is deliberately no fixed "visibleRows" count.
         */
         /*
          * Sticky console header.
          *
          * If scrolling has moved past a console header, keep that
          * header attached to the top of the list viewport while its
          * games continue scrolling underneath.
          *
          * The next console header can push the current sticky header
          * upward, exactly like a sticky section heading in a long list.
          */
         long stickySectionRow = -1;

         if (
             firstRow >= 0 &&
             firstRow <
                 static_cast<long>(rows.size()) &&
             rows[
                 static_cast<std::size_t>(
                     firstRow
                 )
             ].type ==
                 UiRowType::Favorite
         ) {
             for (
                 long i = firstRow - 1;
                 i >= 0;
                 --i
             ) {
                 if (
                     rows[
                         static_cast<std::size_t>(i)
                     ].type ==
                         UiRowType::SystemDivider
                 ) {
                     stickySectionRow = i;
                     break;
                 }
             }
         }

         const int stickyDividerHeight =
             divider
                 ? std::min(
                     fallbackDividerHeight,
                     divider->h
                 )
                 : fallbackDividerHeight;

         const int sectionVisualHeight =
             sectionTopGap +
             sectionTitleHeight +
             stickyDividerHeight;

         /*
          * Draw one console header using the same renderer as the
          * normal list header.
          *
          * This keeps the theme-controlled typography and the existing
          * divider/fallback behavior in one place.
          */
         auto drawSectionHeader =
             [&](const UiRow& row, int topY) {
                 const int titleY =
                     topY + sectionTopGap;

                 drawTextCenteredVertically(
                     screen,
                     sectionFont,
                     row.text,
                     sectionColor,
                     20,
                     titleY,
                     sectionTitleHeight
                 );

                 const int dividerY =
                     titleY + sectionTitleHeight;

                 /*
                  * Theme divider first.
                  *
                  * Some themes contain a transparent divider asset,
                  * so the fallback line below remains necessary.
                  */
                 if (divider) {
                     SDL_Rect sourceRect {
                         0,
                         0,
                         std::min(
                             640,
                             divider->w
                         ),
                         std::min(
                             fallbackDividerHeight,
                             divider->h
                         )
                     };

                     SDL_Rect targetRect {
                         0,
                         dividerY,
                         sourceRect.w,
                         sourceRect.h
                     };

                     SDL_BlitSurface(
                         divider,
                         &sourceRect,
                         screen,
                         &targetRect
                     );
                 }

                 const Uint32 dividerColor =
                     SDL_MapRGB(
                         screen->format,
                         static_cast<Uint8>(
                             theme.currentPageRed
                         ),
                         static_cast<Uint8>(
                             theme.currentPageGreen
                         ),
                         static_cast<Uint8>(
                             theme.currentPageBlue
                         )
                     );

                 SDL_Rect fallbackRect {
                     0,
                     dividerY,
                     640,
                     fallbackDividerHeight
                 };

                 SDL_FillRect(
                     screen,
                     &fallbackRect,
                     dividerColor
                 );

                 /*
                  * Draw the theme asset again so visible theme artwork
                  * wins over the fallback line.
                  */
                 if (divider) {
                     SDL_Rect sourceRect {
                         0,
                         0,
                         std::min(
                             640,
                             divider->w
                         ),
                         std::min(
                             fallbackDividerHeight,
                             divider->h
                         )
                     };

                     SDL_Rect targetRect {
                         0,
                         dividerY,
                         sourceRect.w,
                         sourceRect.h
                     };

                     SDL_BlitSurface(
                         divider,
                         &sourceRect,
                         screen,
                         &targetRect
                     );
                 }
             };

         /*
          * The first visible row still starts at the normal list top.
          *
          * The sticky header is drawn as an overlay later. This means
          * the game rows continue to use the exact existing geometry.
          */
          int y =
              stickySectionRow >= 0
                  ? contentTop + sectionVisualHeight
                  : contentTop;

         for (
             long rowNumber = firstRow;
             rowNumber <
                 static_cast<long>(
                     rows.size()
                 );
             ++rowNumber
         ) {
             const UiRow& row =
                 rows[
                     static_cast<std::size_t>(
                         rowNumber
                     )
                 ];

             const int currentHeight =
                 rowHeight(row);

             if (
                 y >= contentBottom
             ) {
                 break;
             }

             if (
                 y + currentHeight >
                 contentBottom
             ) {
                 break;
             }

             /*
              * Console group heading.
              */
             if (
                 row.type ==
                 UiRowType::SystemDivider
             ) {
                 /*
                  * The current sticky section is already rendered
                  * separately at the top. It must not be rendered a
                  * second time in its original position.
                  */
                 if (
                     rowNumber ==
                     stickySectionRow
                 ) {
                     y += currentHeight;
                     continue;
                 }

                 drawSectionHeader(
                     row,
                     y
                 );

                 y += currentHeight;
                 continue;
             }

             /*
              * Normal Favorite row.
              */
             const bool selected =
                 static_cast<std::size_t>(
                     rowNumber
                 ) == selectedRow;

             /*
              * Onion's selected list background is normally the
              * 56px bg-list-s asset inside a 60px logical row.
              *
              * Preserve the existing theme geometry.
              */
             if (
                 selected &&
                 listSmall
             ) {
                 SDL_Rect selectedRect {
                     0,
                     y +
                         (
                             gameRowHeight -
                             listSmall->h
                         ) / 2,
                     listSmall->w,
                     listSmall->h
                 };

                 SDL_BlitSurface(
                     listSmall,
                     nullptr,
                     screen,
                     &selectedRect
                 );
             }

             drawTextCenteredVertically(
                 screen,
                 listFont,
                 parser.displayLabel(
                     *row.favorite
                 ),
                 selected
                     ? selectedTextColor
                     : listColor,
                 20,
                 y,
                 gameRowHeight
             );

             y += gameRowHeight;
         }

         /*
          * Onion-style selected game preview.
          *
          * preview-bg.png is theme-controlled. The actual artwork
          * comes directly from Favorite::imagePath.
          *
          * Standard Onion geometry:
          * - preview starts at y=60
          * - preview width is the theme asset width
          * - artwork is centered around y=240
          * - artwork is scaled down proportionally when necessary
          */
          SDL_Rect selectedPreviewRect {
              0,
              0,
              0,
              0
          };

          bool selectedPreviewRectValid = false;

         if (previewBackground) {
             const int previewWidth =
                 previewBackground->w;

             SDL_Rect previewBackgroundRect {
                 640 -
                     previewBackground->w,
                 60,
                 previewBackground->w,
                 previewBackground->h
             };

             SDL_BlitSurface(
                 previewBackground,
                 nullptr,
                 screen,
                 &previewBackgroundRect
             );

             if (selectedPreview) {
                 int previewDrawWidth =
                     selectedPreview->w;

                 int previewDrawHeight =
                     selectedPreview->h;

                 if (
                     previewDrawWidth >
                     previewWidth
                 ) {
                     const double scale =
                         static_cast<double>(
                             previewWidth
                         ) /
                         static_cast<double>(
                             selectedPreview->w
                         );

                     previewDrawWidth =
                         static_cast<int>(
                             selectedPreview->w *
                             scale
                         );

                     previewDrawHeight =
                         static_cast<int>(
                             selectedPreview->h *
                             scale
                         );
                 }

                 SDL_Rect previewRect {
                     640 -
                         previewWidth +
                         (
                             previewWidth -
                             previewDrawWidth
                         ) / 2,
                     240 -
                         previewDrawHeight / 2,
                     previewDrawWidth,
                     previewDrawHeight
                 };

                 selectedPreviewRect =
                     previewRect;

                 selectedPreviewRectValid =
                     true;

                 if (
                     previewDrawWidth ==
                         selectedPreview->w &&
                     previewDrawHeight ==
                         selectedPreview->h
                 ) {
                     SDL_BlitSurface(
                         selectedPreview,
                         nullptr,
                         screen,
                         &previewRect
                     );
                 }
                 else {
                     SDL_BlitScaled(
                         selectedPreview,
                         nullptr,
                         screen,
                         &previewRect
                     );
                 }
             }
         }


         /*
          * Draw the sticky console header only after the normal list
          * content and selected-game preview have been rendered.
          *
          * Keep the proven list/sticky geometry unchanged. The only
          * preview-specific behavior is horizontal clipping when the
          * actual rendered artwork overlaps the sticky header.
          */
         if (stickySectionRow >= 0) {
             SDL_Rect stickyBackground {
                 0,
                 contentTop,
                 width,
                 sectionVisualHeight
             };

             SDL_Rect previousClip;

             SDL_GetClipRect(
                 screen,
                 &previousClip
             );

             SDL_Rect headerClip =
                 stickyBackground;

             bool clipStickyHeader = false;

             if (selectedPreviewRectValid) {
                 SDL_Rect overlap;

                 if (
                     SDL_IntersectRect(
                         &stickyBackground,
                         &selectedPreviewRect,
                         &overlap
                     )
                 ) {
                     headerClip.w =
                         std::max(
                             0,
                             overlap.x -
                                 stickyBackground.x
                         );

                     clipStickyHeader = true;
                 }
             }

             if (clipStickyHeader) {
                 SDL_SetClipRect(
                     screen,
                     &headerClip
                 );
             }

             /*
              * Restore the normal theme background underneath the
              * sticky header. The clip prevents this from erasing
              * the selected artwork on the right.
              */
             SDL_BlitSurface(
                 background,
                 &stickyBackground,
                 screen,
                 &stickyBackground
             );

             /*
              * The same clip applies to the title, fallback divider,
              * and themed divider, so all of them stop at exactly the
              * same artwork edge.
              */
             drawSectionHeader(
                 rows[
                     static_cast<std::size_t>(
                         stickySectionRow
                     )
                 ],
                 contentTop
             );

             SDL_SetClipRect(
                 screen,
                 &previousClip
             );
         }

        /*
         * Onion-calibrated footer.
         *
         * Firmware/reference geometry:
         * - footer: y=420..479
         * - first element x=20
         * - button center y=450
         * - label center y=449
         * - icon/label gap: 5px
         * - gap after each label: 30px
         * - counter right edge: x=620
         * - footer text size: 25px
         */
        if (footerBackground) {
            SDL_Rect footerRect {
                0,
                420,
                640,
                60
            };

            blitScaled(
                footerBackground,
                screen,
                footerRect
            );
        }

        if (footerFont) {
            const SDL_Color hintColor {
                static_cast<Uint8>(
                    theme.hint.red
                ),
                static_cast<Uint8>(
                    theme.hint.green
                ),
                static_cast<Uint8>(
                    theme.hint.blue
                ),
                255
            };

            const SDL_Color currentColor {
                static_cast<Uint8>(
                    theme.currentPageRed
                ),
                static_cast<Uint8>(
                    theme.currentPageGreen
                ),
                static_cast<Uint8>(
                    theme.currentPageBlue
                ),
                255
            };

            const SDL_Color totalColor {
                static_cast<Uint8>(
                    theme.totalRed
                ),
                static_cast<Uint8>(
                    theme.totalGreen
                ),
                static_cast<Uint8>(
                    theme.totalBlue
                ),
                255
            };

            /*
             * Draw text with its rendered surface centered on an
             * absolute screen Y coordinate.
             *
             * Returns the rendered width so the next footer item
             * can follow it exactly like Onion.
             */
            auto drawFooterText =
                [&](const std::string& text,
                    const SDL_Color& color,
                    int x,
                    int centerY) -> int {
                    SDL_Surface* rendered =
                        TTF_RenderUTF8_Blended(
                            footerFont,
                            text.c_str(),
                            color
                        );

                    if (!rendered) {
                        return 0;
                    }

                    SDL_Rect target {
                        x,
                        centerY - rendered->h / 2,
                        rendered->w,
                        rendered->h
                    };

                    SDL_BlitSurface(
                        rendered,
                        nullptr,
                        screen,
                        &target
                    );

                    const int width =
                        rendered->w;

                    SDL_FreeSurface(
                        rendered
                    );

                    return width;
                };

            constexpr int buttonCenterY = 450;
            constexpr int labelCenterY = 449;
            constexpr int iconTextGap = 5;
            constexpr int labelGap = 30;

            int offsetX = 20;

            /*
             * A icon.
             *
             * Keep the theme asset at native dimensions.
             * Transparent/large placeholder images supplied by
             * themes are intentionally respected.
             */
            if (buttonA) {
                SDL_Rect buttonARect {
                    offsetX,
                    buttonCenterY -
                        buttonA->h / 2,
                    buttonA->w,
                    buttonA->h
                };

                SDL_BlitSurface(
                    buttonA,
                    nullptr,
                    screen,
                    &buttonARect
                );

                offsetX +=
                    buttonA->w +
                    iconTextGap;
            }

            /*
             * SELECT
             */
             if (!theme.hideHints) {
                 offsetX +=
                     drawFooterText(
                         "SELECT",
                         hintColor,
                         offsetX,
                         labelCenterY
                     );

                 offsetX +=
                     labelGap;
             }

            /*
             * B icon.
             */
             if (buttonB) {
                SDL_Rect buttonBRect {
                    offsetX,
                    buttonCenterY -
                        buttonB->h / 2,
                    buttonB->w,
                    buttonB->h
                };

                SDL_BlitSurface(
                    buttonB,
                    nullptr,
                    screen,
                    &buttonBRect
                );

                offsetX +=
                    buttonB->w +
                    iconTextGap;
            }

            /*
             * BACK
             */
             if (!theme.hideHints) {
                 drawFooterText(
                     "BACK",
                     hintColor,
                     offsetX,
                     labelCenterY
                 );
             }

            /*
             * Favorite position counter.
             *
             * MainUI right-aligns the COMPLETE current/total
             * counter against x=620.
             *
             * Current part and total part use separate theme
             * colors.
             */
            if (
                selectedRow < rows.size() &&
                rows[selectedRow].type ==
                    UiRowType::Favorite
            ) {
                const std::size_t current =
                    rows[selectedRow].favoriteIndex + 1;

                const std::size_t total =
                    favorites.size();

                const std::string currentText =
                    std::to_string(current) + "/";

                const std::string totalText =
                    std::to_string(total);

                int totalWidth = 0;
                int totalHeight = 0;

                TTF_SizeUTF8(
                    footerFont,
                    totalText.c_str(),
                    &totalWidth,
                    &totalHeight
                );

                int currentWidth = 0;
                int currentHeight = 0;

                TTF_SizeUTF8(
                    footerFont,
                    currentText.c_str(),
                    &currentWidth,
                    &currentHeight
                );

                constexpr int counterRightEdge =
                    620;

                const int totalX =
                    counterRightEdge -
                    totalWidth;

                const int currentX =
                    totalX -
                    currentWidth;

                drawFooterText(
                    currentText,
                    currentColor,
                    currentX,
                    labelCenterY
                );

                drawFooterText(
                    totalText,
                    totalColor,
                    totalX,
                    labelCenterY
                );
            }
        }

        SDL_UpdateTexture(
            texture,
            nullptr,
            screen->pixels,
            screen->pitch
        );

        SDL_RenderClear(renderer);

        SDL_RenderCopy(
            renderer,
            texture,
            nullptr,
            nullptr
        );

        SDL_RenderPresent(renderer);
    }

    if (navigationSound) {
        Mix_FreeChunk(
            navigationSound
        );
    }

    Mix_CloseAudio();

    if (selectedPreview) {
        SDL_FreeSurface(
            selectedPreview
        );
    }

    if (previewBackground) {
        SDL_FreeSurface(
            previewBackground
        );
    }

    if (divider) {
        SDL_FreeSurface(divider);
    }

    if (listSmall) {
        SDL_FreeSurface(listSmall);
    }

    if (buttonA) {
        SDL_FreeSurface(buttonA);
    }

    if (buttonB) {
        SDL_FreeSurface(buttonB);
    }

    if (footerBackground) {
        SDL_FreeSurface(
            footerBackground
        );
    }

    if (titleBackground) {
        SDL_FreeSurface(
            titleBackground
        );
    }

    SDL_FreeSurface(background);

    TTF_CloseFont(sectionFont);
    TTF_CloseFont(listFont);
    TTF_CloseFont(titleFont);
    TTF_CloseFont(footerFont);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
