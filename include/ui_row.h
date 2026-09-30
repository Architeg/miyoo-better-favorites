#ifndef BETTER_FAVORITES_UI_ROW_H
#define BETTER_FAVORITES_UI_ROW_H

#include "favorites.h"

#include <cstddef>
#include <string>

enum class UiRowType
{
    SystemDivider,
    Favorite
};

struct UiRow
{
    UiRowType type = UiRowType::Favorite;

    std::string text;

    // Valid only when type == UiRowType::Favorite.
    const Favorite* favorite = nullptr;

    // Index among selectable favorite rows only.
    std::size_t favoriteIndex = 0;
};

#endif
