#ifndef BETTER_FAVORITES_NAVIGATION_H
#define BETTER_FAVORITES_NAVIGATION_H

#include "ui_row.h"

#include <cstddef>
#include <vector>

std::size_t firstSelectableRow(
    const std::vector<UiRow>& rows
);

std::size_t nextSelectableRow(
    const std::vector<UiRow>& rows,
    std::size_t current
);

std::size_t previousSelectableRow(
    const std::vector<UiRow>& rows,
    std::size_t current
);

std::size_t nextConsoleRow(
    const std::vector<UiRow>& rows,
    std::size_t current
);

std::size_t previousConsoleRow(
    const std::vector<UiRow>& rows,
    std::size_t current
);

std::size_t consoleHeaderRow(
    const std::vector<UiRow>& rows,
    std::size_t current
);

#endif
