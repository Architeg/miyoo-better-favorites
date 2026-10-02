#include "favorite_removal.h"
#include <cassert>
#include <fstream>
#include <iterator>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
std::string root, path, phase, operation;
std::string read(const std::string& p) { std::ifstream f(p, std::ios::binary); return {std::istreambuf_iterator<char>(f), {}}; }
void write(const std::string& p, const std::string& b) { std::ofstream f(p, std::ios::binary); f << b; assert(f.good()); }
void hook(const char* point) {
    if (phase != point) return;
    if (operation == "replace") { write(path + ".foreign", "unrelated records\n"); assert(rename((path + ".foreign").c_str(), path.c_str()) == 0); }
    if (operation == "append") { std::ofstream f(path, std::ios::app); f << "foreign\n"; }
    if (operation == "corrupt-backup") {
        DIR* dir = opendir(root.c_str()); assert(dir); struct dirent* item;
        while ((item = readdir(dir))) if (std::string(item->d_name).find(".favourite.json.better-favorites-backup.") == 0)
            write(root + "/" + item->d_name, "changed backup\n");
        closedir(dir);
    }
    if (operation == "make-unwritable") assert(chmod(root.c_str(), 0500) == 0);
}
int main() {
    char folder[] = "/tmp/better-favorites-removal-test.XXXXXX"; assert(mkdtemp(folder)); root = folder; path = root + "/favourite.json";
    const std::string first = "{\"label\":\"same\",\"custom\":{\"preserve\":true},\"type\":5}\r\n";
    const std::string selected = "{\"label\":\"same\",\"rompath\":\"/mnt/SDCARD/Emu/FC/../../Roms/FC/36. Faxanadu.zip\",\"extra\":7}\n";
    const std::string tail = "unparsed line preserved\n{\"label\":\"last\",\"extra\":[1,2]}";
    const auto original = first + selected + tail;
    Favorite favorite; favorite.sourceOffset = first.size(); favorite.sourceRecord = selected;
    FavoritesSnapshot snapshot; std::string error, backup;
    const char* protectedFiles[] = {"rom.zip", "cover.png", "save.srm", "recentlist.json", "recentlist-hidden.json"};
    for (const char* name : protectedFiles) write(root + "/" + name, "untouched sentinel");
    write(path, original); assert(readFavoritesSnapshot(path, snapshot, error));
    assert(removeFavorite(path, snapshot, favorite, backup, error));
    assert(read(path) == first + tail && read(backup) == original && !snapshot.valid);
    // Final unterminated record and last favorite both remove cleanly.
    write(path, selected.substr(0, selected.size()-1)); assert(readFavoritesSnapshot(path, snapshot, error));
    favorite.sourceOffset = 0; favorite.sourceRecord = snapshot.bytes;
    assert(removeFavorite(path, snapshot, favorite, backup, error)); assert(read(path).empty());
    favorite.sourceOffset = first.size(); favorite.sourceRecord = selected;
    write(path, original); assert(readFavoritesSnapshot(path, snapshot, error));
    favorite.sourceOffset++; assert(!removeFavorite(path, snapshot, favorite, backup, error)); assert(read(path) == original);
    favorite.sourceOffset--;
    write(path, original + "foreign\n"); assert(!removeFavorite(path, snapshot, favorite, backup, error)); assert(read(path) == original + "foreign\n");
    favoriteRemovalTestHook = hook;
    for (const char* boundary : {"after-backup", "before-publication"}) {
        for (const char* action : {"replace", "append"}) {
            write(path, original); assert(readFavoritesSnapshot(path, snapshot, error));
            phase = boundary; operation = action;
            assert(!removeFavorite(path, snapshot, favorite, backup, error));
            assert(read(path) == (operation == "replace" ? "unrelated records\n" : original + "foreign\n"));
            assert(read(backup) == original);
        }
    }
    // Fail to create backup / fail to stage replacement (test runs unprivileged).
    if (geteuid() != 0) for (const char* boundary : {"before-backup", "after-backup", "before-publication"}) {
        write(path, original); assert(readFavoritesSnapshot(path, snapshot, error));
        phase = boundary; operation = "make-unwritable";
        assert(!removeFavorite(path, snapshot, favorite, backup, error));
        assert(chmod(root.c_str(), 0700) == 0); assert(read(path) == original);
    }
    write(path, original); assert(readFavoritesSnapshot(path, snapshot, error));
    phase = "before-backup-verification"; operation = "corrupt-backup";
    assert(!removeFavorite(path, snapshot, favorite, backup, error));
    assert(read(path) == original);
    phase.clear();
    const auto target = path + ".target"; assert(rename(path.c_str(), target.c_str()) == 0);
    assert(symlink(target.c_str(), path.c_str()) == 0);
    assert(!readFavoritesSnapshot(path, snapshot, error)); assert(read(target) == original);
    unlink(path.c_str()); unlink(target.c_str());
    for (const char* name : protectedFiles) assert(read(root + "/" + name) == "untouched sentinel");
    DIR* dir = opendir(root.c_str()); struct dirent* item;
    while ((item = readdir(dir))) if (std::string(item->d_name) != "." && std::string(item->d_name) != "..") unlink((root + "/" + item->d_name).c_str());
    closedir(dir); rmdir(root.c_str());
    std::cout << "Removal: exact record/unknown fields, duplicate labels, last record, external conflicts, backup/stage/publication failure, symlink refusal, untouched ROM/art/saves/history: PASS\n";
}
