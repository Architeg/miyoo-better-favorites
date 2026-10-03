// Host fixtures use the existing cJSON bridge; the ARM app retains json-c.
#include "favorites_parser.h"
#include "startup_profile.h"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
namespace fs = std::filesystem;
static void write(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream output(path); output << text; assert(output.good());
}
static void field(const std::string& value) {
    std::cout << value.size() << ':' << value;
}
int main(int argc, char** argv) {
    char pattern[] = "/tmp/better-favorites-label-cache.XXXXXX";
    const char* temporary = mkdtemp(pattern); assert(temporary);
    const fs::path card(temporary);
    const auto config = [&](const std::string& id) { return card / "Emu" / id / "config.json"; };
    write(config("GOOD"), "{\"label\":\"Console 日本語\"}");
    write(config("BAD"), "{not-json");
    write(config("WRONG"), "{\"label\":7}");
    write(config("EMPTY"), "{\"label\":\"\"}");
    write(config("ARRAY"), "[]");
    // An unreadable-as-text resource must retain the ID fallback, too.
    fs::create_directories(config("DIRECTORY"));
    const std::vector<std::string> ids = {"GOOD","MISSING","BAD","WRONG","EMPTY","ARRAY","DIRECTORY",""};
    std::string source = "not-json\n\n";
    std::vector<std::size_t> offsets;
    std::vector<std::string> records;
    for (const auto& id : ids) {
        for (int repeat = 0; repeat < 2; ++repeat) {
            const auto launch = id.empty() ? "/mnt/SDCARD/App/other/launch.sh" : "/mnt/SDCARD/Emu/" + id + "/launch.sh";
            const auto record = "{\"label\":\"01. Game\",\"launch\":\"" + launch +
                "\",\"rompath\":\"/mnt/SDCARD/Emu/GB/../../Roms/GB/./Game.gb\","
                "\"imgpath\":\"/mnt/SDCARD/Roms/GB/Imgs/Game.png\",\"extra\":42}\n";
            offsets.push_back(source.size()); records.push_back(record); source += record;
        }
    }
    source += "[1,2]\n{\"label\":\"trailing\"} invalid\n";
    const FavoritesParser parser(card.string()); // Reuse this same parser for all reloads.
    setenv("BETTER_FAVORITES_PROFILE", "1", 1);
    const auto verify = [&](const std::vector<Favorite>& favorites,
                            const std::map<std::string,std::string>& labels) {
        assert(favorites.size() == ids.size() * 2);
        const auto& metric = startup_profile::state().metrics.at("emu.config_read_parse");
#ifdef BETTER_FAVORITES_UNCACHED_REFERENCE
        assert(metric.calls == 14);
#else
        assert(metric.calls == 7); // Seven config paths, regardless of repeated favorites.
#endif
        assert(metric.identities.size() == 7);
        for (std::size_t i = 0; i < favorites.size(); ++i) {
            const auto& favorite = favorites[i]; const auto& id = ids[i/2];
            assert(favorite.systemId == id);
            assert(favorite.systemLabel == (id.empty() ? "Unknown" : labels.at(id)));
            assert(favorite.label == "01. Game");
            assert(favorite.romPath == "/mnt/SDCARD/Emu/GB/../../Roms/GB/./Game.gb");
            assert(favorite.sourceOffset == offsets[i]);
            assert(favorite.sourceRecord == records[i]);
            // Exact output comparison includes all stored/resolved/source-identity fields.
            for (const auto& value : {favorite.label, favorite.launchPath, favorite.romPath,
                 favorite.imagePath, favorite.systemId, favorite.systemLabel, favorite.sourceRecord}) field(value);
            std::cout << favorite.sourceOffset << '\n';
        }
    };
    startup_profile::start();
    verify(parser.loadFavoritesFromText(source), {{"GOOD","Console 日本語"},{"MISSING","MISSING"},
        {"BAD","BAD"},{"WRONG","WRONG"},{"EMPTY","EMPTY"},{"ARRAY","ARRAY"},{"DIRECTORY","DIRECTORY"}});
    // Config changes/repairs/missing-file creation must appear on the next parse.
    write(config("GOOD"), "{\"label\":\"Changed console\"}");
    write(config("MISSING"), "{\"label\":\"New config\"}");
    write(config("BAD"), "{\"label\":\"Repaired\"}");
    write(config("WRONG"), "{\"label\":null}");
    fs::remove(config("EMPTY"));
    write(config("ARRAY"), "{\"label\":\"Array repaired\"}");
    fs::remove(config("DIRECTORY")); write(config("DIRECTORY"), "{\"label\":\"File now\"}");
    const auto favoritesFile = card / "Roms/favourite.json"; write(favoritesFile, source);
    startup_profile::start();
    verify(parser.loadFavorites(favoritesFile.string()), {{"GOOD","Changed console"},{"MISSING","New config"},
        {"BAD","Repaired"},{"WRONG","WRONG"},{"EMPTY","EMPTY"},{"ARRAY","Array repaired"},{"DIRECTORY","File now"}});
    // Valid -> missing/malformed -> fallback, without constructing a new parser.
    fs::remove(config("GOOD")); write(config("MISSING"), "{");
    startup_profile::start();
    verify(parser.loadFavoritesFromText(source), {{"GOOD","GOOD"},{"MISSING","MISSING"},
        {"BAD","Repaired"},{"WRONG","WRONG"},{"EMPTY","EMPTY"},{"ARRAY","Array repaired"},{"DIRECTORY","File now"}});
    startup_profile::start();
    assert(parser.loadFavoritesFromText("malformed\n[]\n").empty());
    assert(parser.loadFavorites((card / "missing.json").string()).empty());
    assert(startup_profile::state().metrics.count("emu.config_read_parse") == 0);
    startup_profile::state().active = false;
    unsetenv("BETTER_FAVORITES_PROFILE");
    fs::remove_all(card); // Only this test's own fixture, not historical artifacts.
    if (argc == 2) {
        // Optional read-only real-card equivalence audit, never a device timing claim.
        setenv("BETTER_FAVORITES_PROFILE", "1", 1);
        startup_profile::start();
        auto favorites = FavoritesParser(argv[1]).loadFavorites(std::string(argv[1]) + "/Roms/favourite.json");
        assert(!favorites.empty());
        std::size_t nonemptySystems = 0;
        for (const auto& favorite : favorites) {
            if (!favorite.systemId.empty()) ++nonemptySystems;
            for (const auto& value : {favorite.label, favorite.launchPath, favorite.romPath,
                 favorite.imagePath, favorite.systemId, favorite.systemLabel, favorite.sourceRecord}) field(value);
            std::cout << favorite.sourceOffset << '\n';
        }
        const auto& metric = startup_profile::state().metrics.at("emu.config_read_parse");
        assert(metric.identities.size() <= nonemptySystems);
#ifdef BETTER_FAVORITES_UNCACHED_REFERENCE
        assert(metric.calls == nonemptySystems);
#else
        assert(metric.calls == metric.identities.size());
#endif
        std::cerr << "Card corpus favorites=" << favorites.size() << " config_calls=" << metric.calls
                  << " unique_paths=" << metric.identities.size() << '\n';
        startup_profile::state().active = false;
    }
    std::cout << "Equivalent labels, source identity, failures and per-reload refresh: PASS\n";
}
