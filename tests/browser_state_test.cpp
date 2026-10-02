#include "browser_state.h"
#include "navigation.h"
#include "ui_rows.h"
#include <cassert>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    char directory[] = "/tmp/better-favorites-browser-test.XXXXXX";
    assert(mkdtemp(directory));
    const std::string path = std::string(directory) + "/state";
    SystemGroup group;
    group.id = "GB";
    group.label = "Game Boy";
    for (const char* name : {"One", "Two", "Three", "Four"}) {
        Favorite f;
        f.label = name; f.systemId = "GB";
        f.launchPath = "/mnt/SDCARD/Emu/GB/launch.sh";
        f.romPath = "/mnt/SDCARD/Emu/GB/../../Roms/GB/./" + std::string(name) + ".gb";
        group.favorites.push_back(f);
    }
    std::vector<SystemGroup> groups{group}; // Own groups outlive row pointers.
    auto rows = buildUiRows(groups);
    std::string error;
    std::size_t selected = 1; long first = 0;
    assert(!restoreBrowserState(path, rows, selected, first, error));
    assert(error.empty() && selected == 1 && first == 0);
    assert(saveBrowserState(path, rows, 3, 2, error));
    assert(restoreBrowserState(path, rows, selected, first, error));
    assert(selected == 3 && first == 2);

    // Page-selected identity and viewport use the existing saved-state protocol.
    auto paged=pageSelectableRow(rows,1,0,1,360,50);
    assert(paged==4);
    assert(saveBrowserState(path,rows,paged,2,error));
    selected=1;first=0;
    assert(restoreBrowserState(path,rows,selected,first,error));
    assert(selected==paged && first==2);
    assert(saveBrowserState(path,rows,3,2,error));

    // Insertion preserves selected and top identities rather than raw indices.
    auto inserted = group.favorites.front(); inserted.romPath = "inserted";
    groups[0].favorites.insert(groups[0].favorites.begin(), inserted);
    rows = buildUiRows(groups);
    assert(restoreBrowserState(path, rows, selected, first, error));
    assert(selected == 4 && first == 3);
    // Removed selected favorite falls back to its prior selectable ordinal.
    groups[0].favorites.erase(groups[0].favorites.begin() + 3);
    rows = buildUiRows(groups);
    assert(restoreBrowserState(path, rows, selected, first, error));
    assert(selected == 3 && rows[selected].favorite->label == "Two");
    assert(first == 3);

    // Header anchor and literal characters round-trip without shell/JSON parsing.
    groups[0].favorites[0].romPath += "\n\"\\";
    rows = buildUiRows(groups);
    assert(saveBrowserState(path, rows, 1, 0, error));
    assert(restoreBrowserState(path, rows, selected, first, error));
    assert(selected == 1 && first == 0);
    auto empty = buildUiRows({});
    assert(!restoreBrowserState(path, empty, selected, first, error));
    std::ofstream(path) << "BetterFavoritesBrowserState1\n999999999999999999999999\n";
    selected = 1; first = 0;
    assert(!restoreBrowserState(path, rows, selected, first, error));
    assert(!error.empty() && selected == 1 && first == 0);
    assert(unlink(path.c_str()) == 0);
    assert(symlink("missing", path.c_str()) == 0);
    assert(!saveBrowserState(path, rows, 1, 0, error));
    assert(unlink(path.c_str()) == 0);
    assert(!saveBrowserState(std::string(directory) + "/missing/state", rows, 1, 0, error));
    assert(rmdir(directory) == 0);
}
