#include "favorites_parser.h"
#include "navigation.h"
#include "ui_row.h"
#include "ui_rows.h"

#include <iostream>

int main()
{
    FavoritesParser parser("/mnt/SDCARD");

    const auto favorites =
        parser.loadFavorites(
            "/mnt/SDCARD/Roms/favourite.json"
        );

    const auto groups =
        parser.groupFavorites(favorites);

    const auto rows =
        buildUiRows(groups);

    const std::size_t first =
        firstSelectableRow(rows);

    std::cout
        << "First selectable row: "
        << first
        << std::endl;

    if (first >= rows.size()) {
        std::cout << "No selectable favorites." << std::endl;
        return 0;
    }

    std::size_t current = first;

    for (int step = 0; step < 35; ++step) {
        const UiRow& row = rows[current];

        std::cout
            << "Step "
            << step
            << ": row "
            << current
            << " -> "
            << row.text
            << std::endl;

        current =
            nextSelectableRow(rows, current);
    }

    return 0;
}
