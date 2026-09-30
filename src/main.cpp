#include "favorites_parser.h"
#include "navigation.h"
#include "theme_loader.h"
#include "ui_row.h"
#include "ui_rows.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>

namespace
{

bool drawText(
    SDL_Surface* destination,
    TTF_Font* font,
    const std::string& text,
    const SDL_Color& color,
    int x,
    int y
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
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

    SDL_Surface* buttonA = nullptr;
    SDL_Surface* buttonB = nullptr;

    if (!theme.buttonAPath.empty()) {
        buttonA =
            IMG_Load(
                theme.buttonAPath.c_str()
            );
    }

    if (!theme.buttonBPath.empty()) {
        buttonB =
            IMG_Load(
                theme.buttonBPath.c_str()
            );
    }

    SDL_Surface* listSmall = nullptr;

    if (!theme.listSmallPath.empty()) {
        listSmall =
            IMG_Load(
                theme.listSmallPath.c_str()
            );
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

    TTF_Font* sectionFont =
        TTF_OpenFont(
            theme.section.fontPath.c_str(),
            std::max(
                1,
                theme.section.size
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

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type != SDL_KEYDOWN) {
                continue;
            }

            switch (event.key.keysym.sym) {
            case SDLK_UP:
                selectedRow =
                    previousSelectableRow(
                        rows,
                        selectedRow
                    );
                break;

            case SDLK_DOWN:
                selectedRow =
                    nextSelectableRow(
                        rows,
                        selectedRow
                    );
                break;

            case SDLK_LCTRL:
            case SDLK_ESCAPE:
                running = false;
                break;

            default:
                break;
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

        drawText(
            screen,
            titleFont,
            "Favorites",
            titleColor,
            24,
            16
        );

        constexpr int contentTop = 62;
        constexpr int contentBottom = 420;

        constexpr int gameRowHeight = 60;
        constexpr int sectionTopGap = 8;
        constexpr int sectionRowHeight = 40;

        /*
         * Maximum logical rows considered around
         * the current selection. Actual screen use
         * depends on whether rows are game rows or
         * shorter section rows.
         */
        constexpr int visibleRows = 8;

        const int halfWindow =
            visibleRows / 2;

        long firstRow =
            static_cast<long>(
                selectedRow
            ) - halfWindow;

        if (firstRow < 0) {
            firstRow = 0;
        }

        const long maxFirst =
            std::max(
                0L,
                static_cast<long>(
                    rows.size()
                ) - visibleRows
            );

        if (firstRow > maxFirst) {
            firstRow = maxFirst;
        }

        for (
            int visible = 0;
            visible < visibleRows;
            ++visible
        ) {
            const long rowNumber =
                firstRow + visible;

            if (
                rowNumber < 0 ||
                rowNumber >=
                    static_cast<long>(
                        rows.size()
                    )
            ) {
                continue;
            }

            const UiRow& row =
                rows[
                    static_cast<std::size_t>(
                        rowNumber
                    )
                ];

            int y = contentTop;

            /*
             * Calculate Y from the real height
             * of every preceding visible row.
             */
            for (
                int i = 0;
                i < visible;
                ++i
            ) {
                const long previousRowNumber =
                    firstRow + i;

                if (
                    previousRowNumber < 0 ||
                    previousRowNumber >=
                        static_cast<long>(
                            rows.size()
                        )
                ) {
                    continue;
                }

                const UiRow& previousRow =
                    rows[
                        static_cast<
                            std::size_t
                        >(
                            previousRowNumber
                        )
                    ];

                    y +=
                        previousRow.type ==
                            UiRowType::SystemDivider
                            ? sectionTopGap + sectionRowHeight
                            : gameRowHeight;
            }

            if (y >= contentBottom) {
                break;
            }

            if (
                row.type ==
                UiRowType::SystemDivider
            ) {
               y += sectionTopGap;
                /*
                 * Dedicated console section row.
                 */
                drawTextCenteredVertically(
                    screen,
                    sectionFont,
                    row.text,
                    sectionColor,
                    20,
                    y,
                    sectionRowHeight - 6
                );

                /*
                 * Always draw a subtle theme-colored divider.
                 * Some themes provide a transparent or extremely
                 * faint div-line-h.png, so the color line guarantees
                 * that console groups remain visually distinct.
                 */
                SDL_Rect dividerRect {
                    28,
                    y + sectionRowHeight - 2,
                    584,
                    2
                };

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

                SDL_FillRect(
                    screen,
                    &dividerRect,
                    dividerColor
                );

                /*
                 * If the theme has its own divider artwork,
                 * layer it over our fallback line.
                 */
                if (divider) {
                    SDL_Rect themedDividerRect {
                        28,
                        y + sectionRowHeight - 3,
                        584,
                        4
                    };

                    blitScaled(
                        divider,
                        screen,
                        themedDividerRect
                    );
                }

                continue;
            }

            if (
                y + gameRowHeight >
                contentBottom
            ) {
                break;
            }

            const bool selected =
                static_cast<std::size_t>(
                    rowNumber
                ) == selectedRow;

            SDL_Rect itemRect {
                0,
                y,
                640,
                gameRowHeight
            };

            if (selected) {
                if (listSmall) {
                    /*
                     * Onion draws bg-list-s at its native size,
                     * horizontally from x=0 and vertically centered
                     * inside the 60px game-row band.
                     */
                    SDL_Rect selectedRect {
                        0,
                        y + (gameRowHeight - listSmall->h) / 2,
                        listSmall->w,
                        listSmall->h
                    };

                    SDL_BlitSurface(
                        listSmall,
                        nullptr,
                        screen,
                        &selectedRect
                    );
                } else {
                    const Uint32 fallbackColor =
                        SDL_MapRGB(
                            screen->format,
                            static_cast<Uint8>(
                                theme.selectedRed
                            ),
                            static_cast<Uint8>(
                                theme.selectedGreen
                            ),
                            static_cast<Uint8>(
                                theme.selectedBlue
                            )
                        );

                    SDL_FillRect(
                        screen,
                        &itemRect,
                        fallbackColor
                    );
                }
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
                28,
                y,
                gameRowHeight
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

        /*
         * MainUI uses a fixed ~25px footer text size even when
         * config.json specifies a different hint.size.
         * Font family and colors still come from the active theme.
         */
         TTF_Font* footerFont =
             TTF_OpenFont(
                 theme.hint.fontPath.c_str(),
                 std::max(
                     1,
                     theme.hint.size
                 )
             );

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
            if (!theme.hideIcons && buttonA) {
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
             if (!theme.hideIcons && buttonB) {
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

            TTF_CloseFont(
                footerFont
            );
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

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
