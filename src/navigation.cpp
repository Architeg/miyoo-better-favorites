#include "navigation.h"

std::size_t firstSelectableRow(
    const std::vector<UiRow>& rows
)
{
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].type == UiRowType::Favorite) {
            return i;
        }
    }

    return rows.size();
}

std::size_t nextSelectableRow(
    const std::vector<UiRow>& rows,
    std::size_t current
)
{
    if (rows.empty()) {
        return rows.size();
    }

    for (
        std::size_t i = current + 1;
        i < rows.size();
        ++i
    ) {
        if (rows[i].type == UiRowType::Favorite) {
            return i;
        }
    }

    // Already at the last selectable item.
    return current;
}

std::size_t previousSelectableRow(
    const std::vector<UiRow>& rows,
    std::size_t current
)
{
    if (rows.empty() || current == 0) {
        return current;
    }

    std::size_t i = current;

    while (i > 0) {
        --i;

        if (rows[i].type == UiRowType::Favorite) {
            return i;
        }
    }

    // Already at the first selectable item.
    return current;
}
