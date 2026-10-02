#ifndef BETTER_FAVORITES_NAVIGATION_H
#define BETTER_FAVORITES_NAVIGATION_H

#include "ui_row.h"

#include <cstddef>
#include <vector>

std::size_t selectableRowAtOrdinal(const std::vector<UiRow>& rows, std::size_t ordinal);

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

// Pixel geometry shared with the browser renderer; sticky headings consume space.
int browserRowHeight(const UiRow& row, int headingHeight);
std::size_t pageSelectableRow(const std::vector<UiRow>& rows, std::size_t current,
    long firstRow, int direction, int contentHeight, int headingHeight, bool repeated = false);

#endif
