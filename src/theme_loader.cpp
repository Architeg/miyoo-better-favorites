#include "theme_loader.h"

#include <json.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <utility>

ThemeLoader::ThemeLoader(std::string sdRoot)
    : sdRoot_(std::move(sdRoot))
{
}

std::string ThemeLoader::mapSdPath(
    const std::string& path
) const
{
    static const std::string prefix = "/mnt/SDCARD";

    if (path.rfind(prefix, 0) == 0) {
        return sdRoot_ + path.substr(prefix.size());
    }

    return path;
}

std::string ThemeLoader::readActiveThemePath() const
{
    const std::string activeThemeFile =
        sdRoot_ + "/.tmp_update/config/active_theme";

    std::ifstream input(activeThemeFile);

    if (!input.is_open()) {
        return {};
    }

    std::string path;
    std::getline(input, path);

    if (path.empty()) {
        return {};
    }

    while (
        !path.empty() &&
        (
            path.back() == '\r' ||
            path.back() == '\n' ||
            path.back() == ' ' ||
            path.back() == '\t'
        )
    ) {
        path.pop_back();
    }

    std::string mapped =
        mapSdPath(path);

    while (
        mapped.size() > 1 &&
        mapped.back() == '/'
    ) {
        mapped.pop_back();
    }

    return mapped;
}

std::string ThemeLoader::resolveThemePath(
    const std::string& themeRoot,
    const std::string& path
) const
{
    if (path.empty()) {
        return {};
    }

    if (path.rfind("/mnt/SDCARD", 0) == 0) {
        return mapSdPath(path);
    }

    if (!path.empty() && path.front() == '/') {
        return path;
    }

    if (
        !themeRoot.empty() &&
        themeRoot.back() == '/'
    ) {
        return themeRoot + path;
    }

    return themeRoot + "/" + path;
}

Theme ThemeLoader::load() const
{
    Theme theme;

    theme.rootPath =
        readActiveThemePath();

    theme.regularFontRoots.push_back(sdRoot_+"/Saves/CurrentProfile/theme");
    if(!theme.rootPath.empty())theme.regularFontRoots.push_back(theme.rootPath);
    if(theme.rootPath.empty())theme.rootPath=sdRoot_+"/miyoo/app";

    const std::string skinRoot =
        theme.rootPath + "/skin";

    theme.backgroundPath =
        skinRoot + "/background.png";

    theme.titleBackgroundPath =
        skinRoot + "/bg-title.png";

    theme.footerBackgroundPath =
        skinRoot + "/tips-bar-bg.png";

    theme.selectedItemPath =
        skinRoot + "/bg-game-item-f.png";

        /*
         * Onion-style theme asset lookup:
         *
         * 1. Current profile override
         * 2. Active theme
         * 3. Onion/Miyoo fallback theme
         *
         * The renderer should not need to know where an asset came from.
         */
        auto resolveThemeAsset =
            [&](const std::string& filename) {
                const std::string profilePath =
                    sdRoot_ +
                    "/Saves/CurrentProfile/theme/skin/" +
                    filename;

                const std::string activePath=theme.rootPath+"/skin/"+filename;
                const std::string fallbackPath=sdRoot_+"/miyoo/app/skin/"+filename;
                std::vector<std::string> candidates;
                for(const auto& path:{profilePath,activePath,fallbackPath})
                    if(std::find(candidates.begin(),candidates.end(),path)==candidates.end())candidates.push_back(path);
                std::string resolved=fallbackPath;
                for(const auto& path:candidates)if(std::ifstream(path).good()){resolved=path;break;}
                theme.imageCandidates[resolved]=candidates;
                return resolved;
            };

        theme.backgroundPath =
            resolveThemeAsset(
                "background.png"
            );

        theme.titleBackgroundPath =
            resolveThemeAsset(
                "bg-title.png"
            );

        theme.footerBackgroundPath =
            resolveThemeAsset(
                "tips-bar-bg.png"
            );

        theme.previewBackgroundPath =
            resolveThemeAsset(
                "preview-bg.png"
            );

        theme.selectedItemPath =
            resolveThemeAsset(
                "bg-game-item-f.png"
            );

        theme.normalItemPath =
            resolveThemeAsset(
                "bg-game-item-n.png"
            );

        // A missing popup must derive its surface from the browser theme,
        // not pull an unrelated stock popup/selection style into the menu.
        for (const std::string& base : {sdRoot_ + "/Saves/CurrentProfile/theme", theme.rootPath}) {
            for (const char* name : {"bg-pop-menu-4.png", "menu-sub-bg.png"}) {
                const std::string candidate = base + "/skin/" + name;
                if (theme.actionMenuPath.empty() && std::ifstream(candidate).good()) theme.actionMenuPath = candidate;
            }
        }

        for (const std::string& base : {sdRoot_ + "/Saves/CurrentProfile/theme", theme.rootPath}) {
            const std::string candidate = base + "/skin/pop-bg.png";
            if (theme.dialogPath.empty() && std::ifstream(candidate).good()) theme.dialogPath = candidate;
        }

        // Popup/dialog resources retain their established profile/active-only
        // policy. Corrupt overrides can still use a compatible active material.
        for(const auto& selected:{theme.actionMenuPath,theme.dialogPath})if(!selected.empty()){
            std::vector<std::string> candidates;
            for(const auto& base:{sdRoot_+"/Saves/CurrentProfile/theme",theme.rootPath}){
                const std::vector<std::string> names=selected==theme.dialogPath
                    ?std::vector<std::string>{"pop-bg.png"}
                    :std::vector<std::string>{"bg-pop-menu-4.png","menu-sub-bg.png"};
                for(const auto& name:names)candidates.push_back(base+"/skin/"+name);
            }
            theme.imageCandidates[selected]=candidates;
        }

        theme.listSmallPath =
            resolveThemeAsset(
                "bg-list-s.png"
            );

        theme.actionSelectionPath = theme.listSmallPath;
        theme.menuLeftArrowPath = resolveThemeAsset("icon-left-arrow-24.png");
        theme.menuRightArrowPath = resolveThemeAsset("icon-right-arrow-24.png");

        theme.listLargePath =
            resolveThemeAsset(
                "bg-list-l.png"
            );

        theme.horizontalDividerPath =
            resolveThemeAsset(
                "div-line-h.png"
            );

        theme.buttonAPath =
            resolveThemeAsset(
                "icon-A-54.png"
            );

        theme.buttonBPath =
            resolveThemeAsset(
                "icon-B-54.png"
            );

            /*
             * Onion-style configuration fallback:
             *
             * 1. Onion/Miyoo fallback configuration
             * 2. Active theme configuration
             * 3. Current-profile configuration overrides
             *
             * The fallback config establishes the baseline. The active
             * theme then overrides only the values it defines. Finally,
             * profile overrides modify only explicitly supplied values.
             */

            auto loadConfigFile =
                [&](const std::string& path,
                    const ThemeTextStyle* hintFallback,
                    const ThemeTextStyle* listFallback,
                    bool allowFallbacks) {
                    std::ifstream input(path);

                    if (!input.is_open()) {
                        return;
                    }

                    std::ostringstream buffer;
                    buffer << input.rdbuf();

                    json_object* root =
                        json_tokener_parse(
                            buffer.str().c_str()
                        );

                    if (!root) {
                        return;
                    }

                    auto parseColor =
                        [](const char* text,
                           int& red,
                           int& green,
                           int& blue) {
                            if (!text || text[0] != '#') {
                                return;
                            }

                            const std::string value(text);

                            if (value.size() != 7) {
                                return;
                            }

                            char* end = nullptr;

                            const long parsed =
                                std::strtol(
                                    value.c_str() + 1,
                                    &end,
                                    16
                                );

                            if (!end || *end != '\0') {
                                return;
                            }

                            red =
                                static_cast<int>(
                                    (parsed >> 16) & 0xff
                                );

                            green =
                                static_cast<int>(
                                    (parsed >> 8) & 0xff
                                );

                            blue =
                                static_cast<int>(
                                    parsed & 0xff
                                );
                        };

                    auto loadTextStyle =
                        [&](const char* key,
                            ThemeTextStyle& style,
                            const ThemeTextStyle* fallback) {
                            json_object* section = nullptr;

                            if (
                                !json_object_object_get_ex(
                                    root,
                                    key,
                                    &section
                                ) ||
                                !section ||
                                !json_object_is_type(
                                    section,
                                    json_type_object
                                )
                            ) {
                                if (fallback) {
                                    style = *fallback;
                                }

                                return;
                            }

                            if (fallback) {
                                style = *fallback;
                            }

                            json_object* value = nullptr;

                            if (
                                json_object_object_get_ex(
                                    section,
                                    "font",
                                    &value
                                ) &&
                                value &&
                                json_object_is_type(
                                    value,
                                    json_type_string
                                )
                            ) {
                                style.fontPath =
                                    resolveThemePath(
                                        path.substr(0,path.find_last_of('/')),
                                        json_object_get_string(
                                            value
                                        )
                                    );
                                style.fontCandidates.insert(style.fontCandidates.begin(),style.fontPath);
                            }

                            value = nullptr;

                            if (
                                json_object_object_get_ex(
                                    section,
                                    "size",
                                    &value
                                ) &&
                                value
                            ) {
                                style.size =
                                    json_object_get_int(value);
                            }

                            value = nullptr;

                            if (
                                json_object_object_get_ex(
                                    section,
                                    "color",
                                    &value
                                ) &&
                                value &&
                                json_object_is_type(
                                    value,
                                    json_type_string
                                )
                            ) {
                                parseColor(
                                    json_object_get_string(
                                        value
                                    ),
                                    style.red,
                                    style.green,
                                    style.blue
                                );
                            }
                        };

                    /*
                     * hideLabels is part of Onion's theme configuration.
                     */
                    json_object* hideLabels = nullptr;

                    if (
                        json_object_object_get_ex(
                            root,
                            "hideLabels",
                            &hideLabels
                        ) &&
                        hideLabels &&
                        json_object_is_type(
                            hideLabels,
                            json_type_object
                        )
                    ) {
                        json_object* value = nullptr;

                        if (
                            json_object_object_get_ex(
                                hideLabels,
                                "icons",
                                &value
                            ) &&
                            value
                        ) {
                            theme.hideIcons =
                                json_object_get_boolean(value);
                        }

                        value = nullptr;

                        if (
                            json_object_object_get_ex(
                                hideLabels,
                                "hints",
                                &value
                            ) &&
                            value
                        ) {
                            theme.hideHints =
                                json_object_get_boolean(value);
                        }
                    }
                    else {
                        /*
                         * Onion also supports the older hideIconTitle
                         * configuration.
                         */
                        json_object* value = nullptr;

                        if (
                            json_object_object_get_ex(
                                root,
                                "hideIconTitle",
                                &value
                            ) &&
                            value
                        ) {
                            const bool hidden =
                                json_object_get_boolean(value);

                            theme.hideIcons = hidden;
                            theme.hideHints = hidden;
                        }
                    }

                    loadTextStyle(
                        "title",
                        theme.title,
                        allowFallbacks ? nullptr : nullptr
                    );

                    if (allowFallbacks) {
                        loadTextStyle(
                            "hint",
                            theme.hint,
                            &theme.title
                        );

                        loadTextStyle(
                            "list",
                            theme.list,
                            &theme.title
                        );
                    }
                    else {
                        loadTextStyle(
                            "hint",
                            theme.hint,
                            hintFallback
                        );

                        loadTextStyle(
                            "list",
                            theme.list,
                            listFallback
                        );
                    }

                    auto loadSectionColor =
                        [&](const char* sectionName,
                            const char* colorKey,
                            int& red,
                            int& green,
                            int& blue) {
                            json_object* section = nullptr;

                            if (
                                !json_object_object_get_ex(
                                    root,
                                    sectionName,
                                    &section
                                ) ||
                                !section ||
                                !json_object_is_type(
                                    section,
                                    json_type_object
                                )
                            ) {
                                return;
                            }

                            json_object* value = nullptr;

                            if (
                                json_object_object_get_ex(
                                    section,
                                    colorKey,
                                    &value
                                ) &&
                                value &&
                                json_object_is_type(
                                    value,
                                    json_type_string
                                )
                            ) {
                                parseColor(
                                    json_object_get_string(
                                        value
                                    ),
                                    red,
                                    green,
                                    blue
                                );
                            }
                        };

                    loadSectionColor(
                        "currentpage",
                        "color",
                        theme.currentPageRed,
                        theme.currentPageGreen,
                        theme.currentPageBlue
                    );

                    loadSectionColor(
                        "total",
                        "color",
                        theme.totalRed,
                        theme.totalGreen,
                        theme.totalBlue
                    );

                    json_object_put(root);
                };

            /*
             * Establish Onion's fallback configuration first.
             */
            theme.title.fontPath =
                sdRoot_ +
                "/miyoo/app/Exo-2-Bold-Italic.ttf";

            theme.title.fontCandidates={theme.title.fontPath};
            theme.title.size = 36;

            theme.hint = theme.title;
            theme.hint.size = 40;

            theme.list = theme.title;
            theme.list.size = 24;

            theme.currentPageRed = 255;
            theme.currentPageGreen = 255;
            theme.currentPageBlue = 255;

            theme.totalRed = 255;
            theme.totalGreen = 255;
            theme.totalBlue = 255;

            const std::string fallbackConfigPath =
                sdRoot_ +
                "/miyoo/app/config.json";

            loadConfigFile(
                fallbackConfigPath,
                nullptr,
                nullptr,
                true
            );

            /*
             * Apply the active theme on top of the fallback.
             *
             * Missing hint/list values inherit from the active
             * title style, matching Onion's config behavior.
             */
            const auto fallbackTitle=theme.title,fallbackHint=theme.hint,fallbackList=theme.list;
            const std::string activeConfigPath =
                theme.rootPath +
                "/config.json";

            if (
                activeConfigPath != fallbackConfigPath
            ) {
                loadConfigFile(
                    activeConfigPath,
                    &theme.title,
                    &theme.title,
                    true
                );
            }

            /*
             * Apply CurrentProfile overrides last.
             * These modify only values explicitly present there.
             */
            auto appendFallback=[](ThemeTextStyle& style,const ThemeTextStyle& lower){
                for(const auto& font:lower.fontCandidates)
                    if(std::find(style.fontCandidates.begin(),style.fontCandidates.end(),font)==style.fontCandidates.end())style.fontCandidates.push_back(font);
            };
            appendFallback(theme.title,fallbackTitle);
            appendFallback(theme.hint,fallbackHint);
            appendFallback(theme.list,fallbackList);
            const std::string profileConfigPath =
                sdRoot_ +
                "/Saves/CurrentProfile/theme/config.json";

            if (
                std::ifstream(profileConfigPath).good()
            ) {
                loadConfigFile(
                    profileConfigPath,
                    nullptr,
                    nullptr,
                    false
                );
            }

            /*
             * Fonts are theme-controlled, but a missing font file
             * must never make Better Favorites fail to start.
             */
            const std::string fallbackFont =
                sdRoot_ +
                "/miyoo/app/Exo-2-Bold-Italic.ttf";

            auto ensureFont =
                [&](ThemeTextStyle& style) {
                    if(style.fontCandidates.empty())style.fontCandidates.push_back(fallbackFont);
                    // Onion v4.3.1-1 theme_loadFont's firmware fallback, used only
                    // after unusable profile/theme/card fonts, never for weight.
                    const std::string firmwareFont="/customer/app/Exo-2-Bold-Italic.ttf";
                    if(std::find(style.fontCandidates.begin(),style.fontCandidates.end(),firmwareFont)==style.fontCandidates.end())style.fontCandidates.push_back(firmwareFont);
                    for(const auto& candidate:style.fontCandidates) {
                        if(std::ifstream(candidate).good()){style.fontPath=candidate;break;}
                    }

                    style.size =
                        std::max(
                            1,
                            style.size
                        );
                };

            ensureFont(theme.title);
            ensureFont(theme.hint);
            ensureFont(theme.list);

            /*
             * Console section headers remain a Better Favorites feature.
             * Their actual visual style is derived from the active
             * theme's list style; layout spacing remains ours.
             */
            theme.section = theme.list;

            theme.section.size =
                std::max(
                    1,
                    theme.list.size - 2
                );

    return theme;
}
