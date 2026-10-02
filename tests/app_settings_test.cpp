#include "settings.h"
#include <cassert>
#include <fstream>
#include <unistd.h>

int main() {
    unsetenv("BETTER_FAVORITES_RETURN_DIR");
    assert(!automaticReturnAvailable());
    char context[] = "/tmp/better-favorites-return.XXXXXX";
    assert(mkdtemp(context));
    assert(setenv("BETTER_FAVORITES_RETURN_DIR", context, 1) == 0);
    assert(automaticReturnAvailable());
    assert(rmdir(context) == 0);
    assert(!automaticReturnAvailable());
    unsetenv("BETTER_FAVORITES_RETURN_DIR");
    char directory[] = "/tmp/better-favorites-settings-test.XXXXXX";
    assert(mkdtemp(directory));
    const std::string path = std::string(directory) + "/settings.conf";
    AppSettings settings;
    std::string error;
    assert(!settings.automaticReturn);
    assert(loadAppSettings(path, settings, error) && !settings.automaticReturn);
    assert(setAutomaticReturn(path, true, settings, error));
    const std::string first = settings.returnGeneration;
    AppSettings loaded;
    assert(loadAppSettings(path, loaded, error));
    assert(loaded.automaticReturn && loaded.returnGeneration == first);
    assert(setAutomaticReturn(path, false, settings, error));
    assert(!settings.automaticReturn && settings.returnGeneration != first);
    const std::string disabled = settings.returnGeneration;
    assert(setAutomaticReturn(path, true, settings, error));
    assert(settings.returnGeneration != first && settings.returnGeneration != disabled);
    std::ofstream(path) << "BetterFavoritesSettings1\n1\nbad-generation\n";
    assert(!loadAppSettings(path, loaded, error) && !loaded.automaticReturn);
    assert(unlink(path.c_str()) == 0);
    assert(symlink("unrelated", path.c_str()) == 0);
    assert(!setAutomaticReturn(path, false, settings, error));
    assert(settings.automaticReturn); // Failed saves leave the UI preference unchanged.
    assert(!loadAppSettings(path, loaded, error) && !loaded.automaticReturn);
    assert(unlink(path.c_str()) == 0);
    assert(!setAutomaticReturn(std::string(directory) + "/missing/settings", false, settings, error));
    assert(rmdir(directory) == 0);
}
