#include "favorites_parser.h"

#include <json.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <utility>
#include <cctype>

FavoritesParser::FavoritesParser(
    std::string sdRoot,
    AppSettings settings
)
    : sdRoot_(std::move(sdRoot)),
      settings_(settings)
{
}

std::string FavoritesParser::mapSdPath(
    const std::string& path
) const
{
    static const std::string prefix = "/mnt/SDCARD";

    if (path.rfind(prefix, 0) == 0) {
        return sdRoot_ + path.substr(prefix.size());
    }

    return path;
}

std::string FavoritesParser::extractSystemId(
    const std::string& launchPath
) const
{
    static const std::string marker = "/Emu/";

    const std::size_t start = launchPath.find(marker);

    if (start == std::string::npos) {
        return {};
    }

    const std::size_t idStart = start + marker.size();
    const std::size_t idEnd = launchPath.find('/', idStart);

    if (idEnd == std::string::npos || idEnd <= idStart) {
        return {};
    }

    return launchPath.substr(idStart, idEnd - idStart);
}

std::string FavoritesParser::resolveSystemLabel(
    const std::string& systemId
) const
{
    if (systemId.empty()) {
        return "Unknown";
    }

    const std::string configPath =
        sdRoot_ + "/Emu/" + systemId + "/config.json";

    std::ifstream input(configPath);

    if (!input.is_open()) {
        return systemId;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    json_object* root =
        json_tokener_parse(buffer.str().c_str());

    if (!root) {
        return systemId;
    }

    json_object* labelObject = nullptr;

    std::string label = systemId;

    if (
        json_object_object_get_ex(
            root,
            "label",
            &labelObject
        ) &&
        labelObject &&
        json_object_is_type(
            labelObject,
            json_type_string
        )
    ) {
        const char* value =
            json_object_get_string(labelObject);

        if (value && *value != '\0') {
            label = value;
        }
    }

    json_object_put(root);

    return label;
}

std::vector<Favorite> FavoritesParser::loadFavorites(
    const std::string& favoritesFile
) const
{
    std::ifstream input(favoritesFile, std::ios::binary);
    if (!input.is_open()) return {};
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return loadFavoritesFromText(buffer.str());
}

std::vector<Favorite> FavoritesParser::loadFavoritesFromText(const std::string& text) const
{
    std::vector<Favorite> favorites;
    std::istringstream input(text);
    std::size_t offset = 0;
    std::string line;

    while (std::getline(input, line)) {
        const std::size_t sourceOffset = offset;
        const std::size_t sourceLength = line.size() + (input.eof() ? 0 : 1);
        offset += sourceLength;
        if (line.empty()) {
            continue;
        }

        json_tokener* tokener = json_tokener_new();
        if (!tokener) continue;
        json_tokener_set_flags(tokener, JSON_TOKENER_STRICT);
        json_object* root = json_tokener_parse_ex(tokener, line.c_str(),
                                                 static_cast<int>(line.size()) + 1);
        const bool complete = json_tokener_get_error(tokener) == json_tokener_success;
        json_tokener_free(tokener);
        if (!root || !complete) {
            if (root) json_object_put(root);
            continue;
        }

        if (!json_object_is_type(root, json_type_object)) {
            json_object_put(root); continue;
        }
        Favorite favorite;
        favorite.sourceOffset = sourceOffset;
        favorite.sourceRecord = text.substr(sourceOffset, sourceLength);

        json_object* value = nullptr;

        if (
            json_object_object_get_ex(
                root,
                "label",
                &value
            ) &&
            value
        ) {
            favorite.label =
                json_object_get_string(value);
        }

        value = nullptr;

        if (
            json_object_object_get_ex(
                root,
                "launch",
                &value
            ) &&
            value
        ) {
            favorite.launchPath =
                json_object_get_string(value);
        }

        value = nullptr;

        if (
            json_object_object_get_ex(
                root,
                "rompath",
                &value
            ) &&
            value
        ) {
            favorite.romPath =
                json_object_get_string(value);
        }

        value = nullptr;

        if (
            json_object_object_get_ex(
                root,
                "imgpath",
                &value
            ) &&
            value
        ) {
            favorite.imagePath =
                json_object_get_string(value);
        }

        favorite.systemId =
            extractSystemId(favorite.launchPath);

        favorite.systemLabel =
            resolveSystemLabel(favorite.systemId);

        json_object_put(root);

        if (!favorite.label.empty()) {
            favorites.push_back(
                std::move(favorite)
            );
        }
    }

    return favorites;
}

std::string FavoritesParser::displayLabel(
    const Favorite& favorite
) const
{
    if (settings_.showNumericPrefixes) {
        return favorite.label;
    }

    const std::string& label = favorite.label;

    std::size_t pos = 0;

    while (
        pos < label.size() &&
        std::isdigit(
            static_cast<unsigned char>(label[pos])
        )
    ) {
        ++pos;
    }

    if (
        pos == 0 ||
        pos >= label.size() ||
        label[pos] != '.'
    ) {
        return label;
    }

    ++pos;

    while (
        pos < label.size() &&
        std::isspace(
            static_cast<unsigned char>(label[pos])
        )
    ) {
        ++pos;
    }

    return label.substr(pos);
}

std::string FavoritesParser::makeSortKey(
    const std::string& label
) const
{
    if (settings_.sortMode == SortMode::OriginalLabel) {
        return label;
    }

    std::size_t pos = 0;

    while (
        pos < label.size() &&
        std::isdigit(
            static_cast<unsigned char>(label[pos])
        )
    ) {
        ++pos;
    }

    if (
        pos > 0 &&
        pos < label.size() &&
        label[pos] == '.'
    ) {
        ++pos;

        while (
            pos < label.size() &&
            std::isspace(
                static_cast<unsigned char>(label[pos])
            )
        ) {
            ++pos;
        }
    } else {
        pos = 0;
    }

    std::string key = label.substr(pos);

    for (char& c : key) {
        const unsigned char value =
            static_cast<unsigned char>(c);

        if (value < 128) {
            c = static_cast<char>(
                std::tolower(value)
            );
        }
    }

    return key;
}

std::vector<SystemGroup> FavoritesParser::groupFavorites(
    const std::vector<Favorite>& favorites
) const
{
    std::map<std::string, SystemGroup> groups;

    for (const Favorite& favorite : favorites) {
        const std::string key =
            favorite.systemId.empty()
                ? "UNKNOWN"
                : favorite.systemId;

        SystemGroup& group = groups[key];

        if (group.id.empty()) {
            group.id = key;

            group.label =
                favorite.systemLabel.empty()
                    ? key
                    : favorite.systemLabel;
        }

        group.favorites.push_back(favorite);
    }

    std::vector<SystemGroup> result;

    for (auto& pair : groups) {
        SystemGroup& group = pair.second;

        std::sort(
            group.favorites.begin(),
            group.favorites.end(),
            [this](const Favorite& a, const Favorite& b) {
                const std::string aKey =
                    makeSortKey(a.label);

                const std::string bKey =
                    makeSortKey(b.label);

                if (aKey == bKey) {
                    return a.label < b.label;
                }

                return aKey < bKey;
            }
        );

        result.push_back(std::move(group));
    }

    std::sort(
        result.begin(),
        result.end(),
        [](const SystemGroup& a, const SystemGroup& b) {
            return a.label < b.label;
        }
    );

    return result;
}
