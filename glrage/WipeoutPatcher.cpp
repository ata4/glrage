#include "WipeoutPatcher.hpp"

#include "Config.hpp"
#include "Logger.hpp"

namespace glrage {

WipeoutPatcher::WipeoutPatcher() :
    RuntimePatcher("Wipeout")
{ }

GameID WipeoutPatcher::gameID() {
    return GameID::Wipeout;
}

bool WipeoutPatcher::applicable(const std::string& fileName) {
    return fileName == m_config.getString("patch_exe", "wipeout");
}

void WipeoutPatcher::apply() {
    // Wipeout 95 apparently has a bug in the keyboard handling, which sometimes
    // produces random crashes when a key is pressed for a prolonged period of time.
    // The devs decided it would be a good idea to tamper with the system settings
    // to reduce the key repeat rate to a minimum in order to "fix" this bug.
    // The screensaver is also disabled for some unrelated reason.
    // Unfortunately, these custom settings are not restored correctly when the
    // game crashes or otherwise exits unexpectedly, much to the annoyance of the user.
    // This patch replaces the SystemParametersInfo function with a stub to disable
    // that behavior.
    m_tmp.clear();
    m_tmp << hookSystemParametersInfoA;
    patch(0x7E0290, m_tmp);

    // Disable bugged alt+tab check that causes a segfault after playing the into
    // video on Windows 10 and possibly older versions as well.
    // GLRage doesn't really need it anyway and the only side effect is that the
    // game continues to run in the background when tabbed.
    patch(0x46AFB3, "00", "01");

    // Disable unskippable title screen, saves a few seconds of wait time.
    if (m_config.getBool("patch_disable_title_screen", false)) {
        patch(0x46B885, "E8 B8 40 00 00", "90 90 90 90 90");
    }
    
    // Disable introductory video.
    if (m_config.getBool("patch_disable_introductory_video", false)) {
        patch(0x46B808, "E8 33 46 00 00", "90 90 90 90 90");
    }
}

BOOL WipeoutPatcher::hookSystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    // do nothing
    return TRUE;
}

}