#include "startup_profile.h"
#include "favorites_parser.h"
#include "browser_model.h"

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

    startup_profile::Scope phase("emu.config_read_parse",configPath.c_str());
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
    startup_profile::Scope phase("favorites.parse_inclusive");
    std::vector<Favorite> favorites;
    // Cache resolved labels (including fallbacks) only for this parse. Reloads
    // must observe config edits, repairs and removals without stale labels.
    std::map<std::string, std::string> systemLabels;
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

        auto systemLabel = systemLabels.find(favorite.systemId);
        if (systemLabel == systemLabels.end()) {
            systemLabel = systemLabels.emplace(
                favorite.systemId,
                resolveSystemLabel(favorite.systemId)
            ).first;
        }
        favorite.systemLabel = systemLabel->second;

        json_object_put(root);

        if (!favorite.label.empty()) {
            favorites.push_back(
                std::move(favorite)
            );
        }
    }

    return favorites;
}

std::string FavoritesParser::displayLabel(const Favorite& favorite) const {return browserDisplayLabel(favorite,settings_);}
std::vector<SystemGroup> FavoritesParser::groupFavorites(const std::vector<Favorite>& favorites) const {return groupBrowserFavorites(favorites,settings_);}
