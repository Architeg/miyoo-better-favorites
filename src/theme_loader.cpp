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

    if (theme.rootPath.empty()) {
        return theme;
    }

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

    theme.normalItemPath =
        skinRoot + "/bg-game-item-n.png";

    theme.listSmallPath =
        skinRoot + "/bg-list-s.png";

    theme.listLargePath =
        skinRoot + "/bg-list-l.png";

    theme.horizontalDividerPath =
        skinRoot + "/div-line-h.png";

    theme.buttonAPath =
        skinRoot + "/icon-A-54.png";

    theme.buttonBPath =
        skinRoot + "/icon-B-54.png";

    const std::string configPath =
        theme.rootPath + "/config.json";

    std::ifstream input(configPath);

    if (!input.is_open()) {
        return theme;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    json_object* root =
        json_tokener_parse(
            buffer.str().c_str()
        );

    if (!root) {
        return theme;
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
            ThemeTextStyle& style) {
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
                return;
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
                        theme.rootPath,
                        json_object_get_string(
                            value
                        )
                    );
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

    loadTextStyle(
        "title",
        theme.title
    );

    loadTextStyle(
        "list",
        theme.list
    );

    loadTextStyle(
        "hint",
        theme.hint
    );

    /*
     * Console section headers inherit the active list theme.
     * We keep the same font and color, but make the section
     * slightly smaller so it reads as a group heading rather
     * than another game row.
     */
    theme.section = theme.list;

    theme.section.size =
        std::max(
            1,
            theme.list.size - 2
        );

    /*
     * Onion theme setting:
     *
     * "hideLabels": {
     *     "icons": true,
     *     "hints": true
     * }
     *
     * These control whether footer button icons and
     * footer hint labels are shown.
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
            value &&
            json_object_is_type(
                value,
                json_type_boolean
            )
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
            value &&
            json_object_is_type(
                value,
                json_type_boolean
            )
        ) {
            theme.hideHints =
                json_object_get_boolean(value);
        }
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
        "grid",
        "selectedcolor",
        theme.selectedRed,
        theme.selectedGreen,
        theme.selectedBlue
    );

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

    return theme;
}
