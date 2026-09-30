#include "theme_loader.h"

#include <iostream>

int main()
{
    ThemeLoader loader("/mnt/SDCARD");

    const Theme theme = loader.load();

    std::cout
        << "Theme root: "
        << theme.rootPath
        << std::endl;

    std::cout
        << "Background: "
        << theme.backgroundPath
        << std::endl;

    std::cout
        << "Title background: "
        << theme.titleBackgroundPath
        << std::endl;

    std::cout
        << "Footer background: "
        << theme.footerBackgroundPath
        << std::endl;

    std::cout
        << "Selected item: "
        << theme.selectedItemPath
        << std::endl;

    std::cout
        << "Normal item: "
        << theme.normalItemPath
        << std::endl;

    std::cout
        << "Divider: "
        << theme.horizontalDividerPath
        << std::endl;

    std::cout
        << "Title font: "
        << theme.title.fontPath
        << " size="
        << theme.title.size
        << std::endl;

    std::cout
        << "List font: "
        << theme.list.fontPath
        << " size="
        << theme.list.size
        << std::endl;

    std::cout
        << "Hint font: "
        << theme.hint.fontPath
        << " size="
        << theme.hint.size
        << std::endl;

    return 0;
}
