#ifndef BETTER_FAVORITES_UI_ROWS_H
#define BETTER_FAVORITES_UI_ROWS_H

#include "favorites.h"
#include "ui_row.h"

#include <vector>

std::vector<UiRow> buildUiRows(
    const std::vector<SystemGroup>& groups
);

#endif
