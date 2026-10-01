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

std::size_t nextConsoleRow(
    const std::vector<UiRow>& rows,
    std::size_t current
)
{
    if (rows.empty() || current >= rows.size()) {
        return current;
    }

    /*
     * Find the next console header, then find its first favorite.
     * Empty console groups are skipped automatically.
     */
    for (
        std::size_t i = current + 1;
        i < rows.size();
        ++i
    ) {
        if (
            rows[i].type !=
            UiRowType::SystemDivider
        ) {
            continue;
        }

        for (
            std::size_t j = i + 1;
            j < rows.size();
            ++j
        ) {
            if (
                rows[j].type ==
                UiRowType::SystemDivider
            ) {
                break;
            }

            if (
                rows[j].type ==
                UiRowType::Favorite
            ) {
                return j;
            }
        }
    }

    // Already in the last console with favorites.
    return current;
}

std::size_t previousConsoleRow(
    const std::vector<UiRow>& rows,
    std::size_t current
)
{
    if (rows.empty() || current == 0) {
        return current;
    }

    /*
     * First locate the console containing the current favorite.
     */
    std::size_t currentConsole = rows.size();

    for (
        std::size_t i = current;
        i > 0;
        --i
    ) {
        if (
            rows[i].type ==
            UiRowType::SystemDivider
        ) {
            currentConsole = i;
            break;
        }
    }

    if (
        currentConsole == rows.size()
    ) {
        return current;
    }

    /*
     * Walk backwards to the previous console header.
     */
    std::size_t previousConsole =
        rows.size();

    for (
        std::size_t i = currentConsole;
        i > 0;
        --i
    ) {
        const std::size_t candidate =
            i - 1;

        if (
            rows[candidate].type ==
            UiRowType::SystemDivider
        ) {
            previousConsole = candidate;
            break;
        }
    }

    if (
        previousConsole == rows.size()
    ) {
        return current;
    }

    /*
     * Find the first favorite belonging to that console.
     * Empty groups are skipped.
     */
    for (
        std::size_t i = previousConsole + 1;
        i < currentConsole;
        ++i
    ) {
        if (
            rows[i].type ==
            UiRowType::Favorite
        ) {
            return i;
        }
    }

    /*
     * The previous header may belong to an empty group.
     * Continue searching further backwards.
     */
    return previousConsoleRow(
        rows,
        previousConsole
    );
}

std::size_t consoleHeaderRow(
    const std::vector<UiRow>& rows,
    std::size_t current
)
{
    if (
        rows.empty() ||
        current >= rows.size()
    ) {
        return rows.size();
    }

    for (
        std::size_t i = current;
        i > 0;
        --i
    ) {
        const std::size_t candidate = i - 1;

        if (
            rows[candidate].type ==
            UiRowType::SystemDivider
        ) {
            return candidate;
        }
    }

    return rows.size();
}
