#include "browser_model.h"
#include "navigation.h"
#include <algorithm>
#include <map>
#include <cctype>
std::string browserDisplayLabel(
    const Favorite& favorite, const AppSettings& settings
)
{
    if (settings.showNumericPrefixes) {
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

std::string browserSortKey(
    const std::string& label, const AppSettings& settings
)
{
    if (settings.sortMode == SortMode::OriginalLabel) {
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

std::vector<SystemGroup> groupBrowserFavorites(
    const std::vector<Favorite>& favorites, const AppSettings& settings
)
{
    auto compare=[&](const Favorite& a,const Favorite& b){const auto ak=browserSortKey(a.label,settings),bk=browserSortKey(b.label,settings);if(ak!=bk)return ak<bk;if(a.label!=b.label)return a.label<b.label;return a.sourceOffset<b.sourceOffset;};
    if(!settings.groupByConsole){if(favorites.empty())return {};SystemGroup flat;flat.favorites=favorites;std::stable_sort(flat.favorites.begin(),flat.favorites.end(),compare);return {flat};}
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

        std::stable_sort(group.favorites.begin(),group.favorites.end(),compare);

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

BrowserAnchor captureBrowserAnchor(const std::vector<UiRow>& rows,std::size_t selected,long first){
    BrowserAnchor a;if(selected>=rows.size()||!rows[selected].favorite)return a;
    a.valid=true;a.selected=*rows[selected].favorite;a.ordinal=rows[selected].favoriteIndex;a.first=first;
    if(first>=0&&static_cast<std::size_t>(first)<rows.size()){
        auto top=static_cast<std::size_t>(first);if(!rows[top].favorite && top+1<rows.size())++top;
        if(rows[top].favorite){a.top=*rows[top].favorite;a.hasTop=true;}
    }return a;
}
void restoreBrowserAnchor(const BrowserAnchor& a,const std::vector<UiRow>& rows,std::size_t& selected,long& first){
    selected=selectableRowAtOrdinal(rows,a.ordinal);first=0;if(rows.empty())return;
    auto match=[&](const Favorite& f){std::size_t result=rows.size();for(std::size_t i=0;i<rows.size();++i)if(rows[i].favorite&&rows[i].favorite->launchPath==f.launchPath&&rows[i].favorite->romPath==f.romPath){if(result==rows.size())result=i;if(rows[i].favorite->sourceOffset==f.sourceOffset)return i;}return result;};
    if(a.valid){const auto found=match(a.selected);if(found<rows.size())selected=found;}
    first=std::max(0L,std::min(a.first,static_cast<long>(selected)));
    if(a.hasTop){const auto top=match(a.top);if(top<rows.size())first=static_cast<long>(std::min(top,selected));}
}
