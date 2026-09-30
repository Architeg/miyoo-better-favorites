#include <SDL.h>
#include <json.h>

#include <iostream>

int main()
{
    const char* text =
        "{\"label\":\"Tetris\",\"launch\":\"/mnt/SDCARD/Emu/GB/launch.sh\"}";

    json_object* root = json_tokener_parse(text);

    if (!root) {
        std::cerr << "JSON parse failed" << std::endl;
        return 1;
    }

    json_object* label = nullptr;

    if (json_object_object_get_ex(root, "label", &label)) {
        std::cout << "Parsed label: "
                  << json_object_get_string(label)
                  << std::endl;
    }

    json_object_put(root);

    return 0;
}
