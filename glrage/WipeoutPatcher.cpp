#include "WipeoutPatcher.hpp"

#include "Config.hpp"
#include "Logger.hpp"

namespace glrage {

WipeoutPatcher::WipeoutPatcher() :
    RuntimePatcher("Wipeout") {
}

bool WipeoutPatcher::applicable(const std::string& fileName) {
    return fileName == m_config.getString("patch_exe", "wipeout");
}

void WipeoutPatcher::apply() {
    // This patch prevents Wipeout from tampering with the system settings, which
    // are not always restored correctly, much to the annoyance of the user.
    m_tmp.clear();
    m_tmp << hookSystemParametersInfoA;
    
    patch(0x7E0290, "30 6C 54 76", m_tmp);

    // Disable title screen.
    patch(0x46B885, "E8 B8 40 00 00", "90 90 90 90 90");
}

BOOL WipeoutPatcher::hookSystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    return TRUE;
}

}