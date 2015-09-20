#include "AssaultRigsPatcher.hpp"

#include "Config.hpp"

#include <Windows.h>

namespace glrage {

AssaultRigsPatcher::AssaultRigsPatcher() :
    MemoryPatcher("Assault Rigs") {
}

bool AssaultRigsPatcher::applicable(const std::string& fileName) {
    return fileName == m_config.getString("patch_exe", "arigs");
}

void AssaultRigsPatcher::apply() {
    // Fix "Insufficient memory" error on systems with more than 4 GB RAM where
    // GlobalMemoryStatus returns -1 (unless compatibility mode is activated, 
    // which would break GLRage).
    patch(0x434B63, "0F 8E 32 01 00 00", "90 90 90 90 90 90");

    // HD/widescreen resolution patch. Replaces 640x480, normally the maximum
    // resolution, with the current desktop resolution.
    if (m_config.getBool("patch_resolution", true)) {
        int32_t width = m_config.getInt("patch_resolution_width", -1);
        int32_t height = m_config.getInt("patch_resolution_height", -1);

        if (width <= 0) {
            width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        }

        if (height <= 0) {
            height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        }

        std::vector<uint8_t> resolution;
        appendBytes(width, resolution);
        appendBytes(height, resolution);
        patch(0x490234, "80 02 00 00 E0 01 00 00", resolution);

        // Replace "640 BY 480" in the options with "CUSTOM".
        patch(0x486AC0, "36 34 30 20 42 59 20 34 38 30", "43 55 53 54 4F 4D 20 20 20 20");
    }
}

}