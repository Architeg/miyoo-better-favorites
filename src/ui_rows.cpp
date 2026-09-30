#include "ui_rows.h"

#include <cstddef>

std::vector<UiRow> buildUiRows(
    const std::vector<SystemGroup>& groups
)
{
    std::vector<UiRow> rows;

    std::size_t favoriteIndex = 0;

    for (const SystemGroup& group : groups) {
        UiRow divider;
        divider.type = UiRowType::SystemDivider;
        divider.text = group.label;

        rows.push_back(divider);

        for (const Favorite& favorite : group.favorites) {
            UiRow row;
            row.type = UiRowType::Favorite;
            row.text = favorite.label;
            row.favorite = &favorite;
            row.favoriteIndex = favoriteIndex;

            rows.push_back(row);

            ++favoriteIndex;
        }
    }

    return rows;
}
