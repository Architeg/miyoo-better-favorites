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
         * Select the first logical row by PIXEL height rather than
         * assuming that eight logical rows fit on screen.
         *
         * This is important because our console headers are not 60px
         * game rows. It also prevents the final rows from collapsing
         * into the same visual position.
         */
        long firstRow =
            static_cast<long>(
                selectedRow
            );

        int accumulatedHeight = 0;

        /*
         * Keep approximately two normal game rows above the selection
         * whenever there is enough content before it.
         */
        constexpr int desiredTopContext = 120;

        while (firstRow > 0) {
            const UiRow& previous =
                rows[
                    static_cast<std::size_t>(
                        firstRow - 1
                    )
                ];

            const int previousHeight =
                rowHeight(previous);

            if (
                accumulatedHeight +
                previousHeight >
                contentBottom - contentTop
            ) {
                break;
            }

            accumulatedHeight +=
                previousHeight;

            --firstRow;

            if (
                accumulatedHeight >=
                desiredTopContext
            ) {
                break;
            }
        }

        /*
         * Make sure the selected row itself fits in the viewport.
         *
         * This is particularly important near the bottom of the
         * Favorites list.
         */
        while (
            firstRow <
            static_cast<long>(selectedRow)
        ) {
            int totalHeight = 0;

            for (
                long i = firstRow;
                i <=
                    static_cast<long>(selectedRow);
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

            if (
                totalHeight <=
                contentBottom - contentTop
            ) {
                break;
            }

            ++firstRow;
        }

        /*
         * Render forward until the physical list area is full.
         *
         * There is deliberately no fixed "visibleRows" count.
         */
        int y = contentTop;

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
                y += sectionTopGap;

                drawTextCenteredVertically(
                    screen,
                    sectionFont,
                    row.text,
                    sectionColor,
                    20,
                    y,
                    sectionTitleHeight
                );

                /*
                 * The dedicated divider belongs AFTER the console
                 * title and BEFORE the first game in the group.
                 *
                 * The theme asset is drawn at its native width when
                 * possible. We do not force it into the old 584px
                 * artificial area.
                 */
                const int dividerY =
                    y + sectionTitleHeight;

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

                /*
                 * Some themes intentionally have a transparent or
                 * effectively invisible div-line-h.png.
                 *
                 * Keep our subtle fallback line, but only where the
                 * theme asset does not visibly provide one.
                 *
                 * We currently retain this fallback because your
                 * mini.os theme uses a transparent divider asset.
                 */
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
                 * Theme artwork is deliberately drawn LAST so that,
                 * when it contains visible pixels, it wins over the
                 * fallback line.
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
             * Preserve the theme asset's native dimensions instead
             * of inventing a replacement geometry.
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

            /*
             * Render the game name.
             *
             * x=20 follows the native Onion list text position.
             */
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
