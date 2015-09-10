#include "AssaultRigsPatcher.hpp"

#include "Config.hpp"

#include <Windows.h>

namespace glrage {

bool AssaultRigsPatcher::applicable(const std::string& fileName) {
    return fileName == "arigs.exe";
}

void AssaultRigsPatcher::apply() {
    // Fix "Insufficient memory" error on systems with more than 4 GB RAM where
    // GlobalMemoryStatus returns -1 (unless compatibility mode is activated, 
    // which would break GLRage).
    //
    // Original instructions:
    // cmp     eax, 400000h ; compare to 4 MiB
    // jle     loc_434C9B   ; jump to error message if less or equal
    patch(0x434B5E, "3D 00 00 40 00 0F 8E 32 01 00 00", "90 90 90 90 90 90 90 90 90 90 90");

    // HD/widescreen resolution patch. Replaces 640x480, normally the maximum
    // resolution, with the current desktop resolution.
    if (Config::getBool("Assault Rigs", "patch_resolution", true)) {
        int32_t width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int32_t height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

        std::vector<uint8_t> resolution;
        appendBytes(width, resolution);
        appendBytes(height, resolution);
        patch(0x490234, "80 02 00 00 E0 01 00 00", resolution);

        // Replace "640 BY 480" in the options with "DESKTOP".
        patch(0x486AC0, "36 34 30 20 42 59 20 34 38 30", "44 45 53 4B 54 4F 50 20 20 20");
    }
}

}