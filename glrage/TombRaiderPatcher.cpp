#include "TombRaiderPatcher.hpp"
#include "TombRaiderHooks.hpp"

#include "StringUtils.hpp"
#include "Logger.hpp"

namespace glrage {

TombRaiderPatcher::TombRaiderPatcher() :
    MemoryPatcher("Tomb Raider")
{
}

bool TombRaiderPatcher::applicable(const std::string& fileName) {
    if (fileName == m_config.getString("patch_exe", "tombati")) {
        m_ub = false;
        return true;
    }

    if (fileName == m_config.getString("patch_exe_ub", "tombub")) {
        m_ub = true;
        return true;
    }

    return false;
}

void TombRaiderPatcher::apply() {
    TombRaiderHooks::m_ub = m_ub;

    // mandatory crash patches
    applyCrashPatches();

    // optional patches
    applyGraphicPatches();
    applySoundPatches();
    applyLogicPatches();
}

void TombRaiderPatcher::applyCrashPatches() {
    // Tomb Raider ATI patch fails on later Windows versions because of a missing
    // return statement in a function.
    // In Windows 95, it is compensated by OutputDebugString's nonzero eax value,
    // while in later Windows versions, OutputDebugString returns zero.
    // Unfinished Business fails even on Windows 95, because it does not call
    // OutputDebugString at all.
    // This 'fix' injects "xor eax,eax; inc eax" into a function calling 
    // OutputDebugString, called from 'bad' function.
    // The 'bad' function can not be fixed directly because of lack of room
    // (not enough NOPs after ret).
    patch(m_ub ? 0x429ED0 : 0x42A2F6, "C3 90 90 90", "31 C0 40 C3");

    // Tihocan Centaurs and the Giant Atlantean crash the game when they explode
    // on death, because EAX is 0 in these cases.
    // This patch disables the bugged part of the routine, which doesn't appear
    // to affect visuals or sounds.
    // I guess it influences the damage, since the parts now deal a lot more
    // damage to Lara, but that's still better than no explosions or even a crash.
    patch(m_ub ? 0x43C288 : 0x43C938, "F6 C3 1C 74", "90 90 90 EB");

    // User keys are heavily bugged in the ATI patch and is the source of
    // several crashes, possibly a result of the Windows port, so disable it
    // entirely instead of creating hundreds of assembly patches to fix it somehow.
    // One can still write a program to change the key bindings in the config file.

    // Prevent selection of user keys in the options (changing them crashes the game).
    patch(m_ub ? 0x42E76C : 0x42EB7C, "0F 94 C0", "90 90 90");

    // Crude fix for a crash when opening the control options while in-game.
    // The "Inventory" label is broken for some reason, so it needs to be skipped.
    if (m_ub) {
        patch(0x42F207, "54", "50");
    } else {
        patch(0x42F6DB, "7C", "78");
    }
}

void TombRaiderPatcher::applyGraphicPatches() {
    // The ATI version of Tomb Raider converts vertex colors to half of the original
    // brightness, which results in a dim look and turns some areas in dark levels
    // almost pitch black. This patch boosts the brightness back to normal levels.
    if (m_config.getBool("patch_brightness", true)) {
        float brightness = m_config.getFloat("patch_brightness_value", 1.0f);
        float divisor = (1.0f / brightness) * 1024;
        float multi = 0.0625f * brightness;

        std::vector<uint8_t> tmp;
        appendBytes(divisor, tmp);

        patch(0x451034, "00 00 00 45", tmp);

        tmp.clear();
        appendBytes(multi, tmp);

        patch(0x45103C, "DB F6 FE 3C", tmp);
    }

    // This patch allows the customization of the water color, which is rather
    // ugly on default.
    if (m_config.getBool("patch_watercolor", true)) {
        float filterRed = m_config.getFloat("patch_watercolor_filter_red", 0.3f);
        float filterGreen = m_config.getFloat("patch_watercolor_filter_green", 1.0f);

        std::vector<uint8_t> tmp;
        appendBytes(filterRed, tmp);
        appendBytes(filterGreen, tmp);

        patch(0x451028, "9A 99 19 3F 33 33 33 3F", tmp);
    }

    // This patch replaces 800x600 with a custom resolution for widescreen
    // support and to reduce vertex artifacts due to subpixel inaccuracy,
    if (m_config.getBool("patch_resolution", true)) {
        int32_t width = m_config.getInt("patch_resolution_width", -1);
        int32_t height = m_config.getInt("patch_resolution_height", -1);

        if (width <= 0) {
            width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        }

        if (height <= 0) {
            height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        }

        std::vector<uint8_t> tmp;

        // update display mode and viewport parameters
        appendBytes(width, tmp);
        patch(m_ub ? 0x407CAA : 0x407C9D, "20 03 00 00", tmp);
        tmp.clear();

        appendBytes(height, tmp);
        patch(m_ub ? 0x407CB4 : 0x407CA7, "58 02 00 00", tmp);
        tmp.clear();

        appendBytes(static_cast<float_t>(width - 1), tmp);
        patch(m_ub ? 0x407CBE : 0x407CB1, "00 C0 47 44", tmp);
        tmp.clear();

        appendBytes(static_cast<float_t>(height - 1), tmp);
        patch(m_ub ? 0x407CC8 : 0x407CBB, "00 C0 15 44", tmp);
        tmp.clear();

        // update clipping size
        appendBytes(static_cast<int16_t>(width), tmp);
        patch(m_ub ? 0x408A64 : 0x408A57, "20 03", tmp);
        tmp.clear();

        appendBytes(static_cast<int16_t>(height), tmp);
        patch(m_ub ? 0x408A6D : 0x408A60, "58 02", tmp);
        tmp.clear();

        // set display string (needs to be static so the data won't vanish after
        // patching has finished)
        static std::string displayMode = StringUtils::format("%dx%d", 24, width, height);

        appendBytes(reinterpret_cast<int32_t>(displayMode.c_str()), tmp);
        if (m_ub) {
            patch(0x42DB5B, "40 61 45 00 ", tmp);
        } else {
            patch(0x42DF6B, "58 67 45 00", tmp);
        }
        tmp.clear();

        // UI scale patch, rescales the in-game overlay to keep the proportions
        // of the 800x600 resolution on higher resolutions.
        TombRaiderHooks::m_tombRenderLine = reinterpret_cast<TombRaiderRenderLine*>(0x402710);
        TombRaiderHooks::m_tombRenderCollectedItem = reinterpret_cast<TombRaiderRenderCollectedItem*>(0x435D80);
        TombRaiderHooks::m_tombCreateOverlayText = reinterpret_cast<TombRaiderCreateOverlayText*>(0x439780);
        TombRaiderHooks::m_tombRenderWidth = reinterpret_cast<int32_t*>(0x6CADD4);

        patchAddr(0x41DD85, "E8 46 25 01 00", TombRaiderHooks::renderHealthBar, 0xE8);
        patchAddr(0x41DF0C, "E8 BF 23 01 00", TombRaiderHooks::renderHealthBar, 0xE8);
        patchAddr(0x41DE71, "E8 0A 7F 01 00", TombRaiderHooks::renderCollectedItem, 0xE8);
        patchAddr(0x439B72, "E8 09 FC FF FF", TombRaiderHooks::createFPSText, 0xE8);
    }

    // Not sure what exactly this value does, but setting it too low sometimes
    // produces wrong vertex positions on the far left and right side of the
    // screen, especially on high resolutions.
    // Raising it from 10 to the maximum value of 127 fixes that.
    std::string tmpExp = "0A";
    std::string tmpRep = "7F";
    patch(m_ub ? 0x4163E9 : 0x4164D9, tmpExp, tmpRep);
    patch(m_ub ? 0x41657A : 0x41666A, tmpExp, tmpRep);
    patch(m_ub ? 0x41666E : 0x41675E, tmpExp, tmpRep);
    patch(m_ub ? 0x416801 : 0x4168F1, tmpExp, tmpRep);
    patch(m_ub ? 0x4168FE : 0x4169EE, tmpExp, tmpRep);

    if (m_config.getBool("patch_draw_distance", false)) {
        int32_t drawDistFade = m_config.getInt("patch_draw_distance_fade", 12288);
        int32_t drawDistMax = m_config.getInt("patch_draw_distance_max", 20480);

        std::vector<uint8_t> drawDistFadeData;
        appendBytes(drawDistFade, drawDistFadeData);

        std::vector<uint8_t> drawDistFadeNegData;
        appendBytes(-drawDistFade, drawDistFadeNegData);

        std::vector<uint8_t> drawDistMaxData;
        appendBytes(drawDistMax, drawDistMaxData);

        patch(0x402030, "00 50 00 00", drawDistMaxData);
        patch(0x402047, "00 30 00 00", drawDistFadeData);
        patch(0x40205A, "00 30 00 00", drawDistFadeData);

        patch(0x40224B, "00 50 00 00", drawDistMaxData);
        patch(0x402263, "00 30 00 00", drawDistFadeData);
        patch(0x402270, "00 D0 FF FF", drawDistFadeNegData);

        patch(m_ub ? 0x4163E4 : 0x4164D4, "00 50 00 00", drawDistMaxData);

        patch(m_ub ? 0x42FD82 : 0x430252, "00 30 00 00", drawDistFadeData);
        patch(m_ub ? 0x42FD91 : 0x430261, "00 D0 FF FF", drawDistFadeNegData);

        patch(m_ub ? 0x42FDDD : 0x4302AD, "00 30 00 00", drawDistFadeData);
        patch(m_ub ? 0x42FDE6 : 0x4302B6, "00 D0 FF FF", drawDistFadeNegData);

        patch(m_ub ? 0x435521 : 0x435AA1, "00 D0 FF FF", drawDistFadeNegData);

        patch(m_ub ? 0x435729 : 0x435CA9, "00 D0 FF FF", drawDistFadeNegData);
    }

    // This patch raises the maximum FPS from 30 to 60.
    // FIXME: disabled, since only actually works in menu while the game itself
    // just renders duplicate frames.
    //if (m_config.getBool("patch_60fps", true)) {
    //    // render on every tick instead of every other
    //    patch(m_ub ? 0x408A91 : 0x408A84, "02", "01");
    //    // disables frame skipping, which also fixes issues with the demo mode
    //    // if the frame rate isn't exactly at limit all the time
    //    patch(m_ub ? 0x408ABA : 0x408AAD, "83 E1 1F", "33 C9 90");
    //}
}

void TombRaiderPatcher::applySoundPatches() {
    // For reasons unknown, the length of a sound sample is stored in a struct
    // field as a 16 bit integer, which means that the maximum allowed length for
    // a sound sample is be 65535 bytes. If a sample is larger, which happens quite
    // often for Lara's speeches in her home, then there's an integer overflow 
    // and the length is wrapped to the modulo of 65535. This causes her speech
    // to cut off, if too long. In one case ("Ah, the main hall..."), the sample
    // is just slightly larger than 64k, which causes the game to play only the
    // first few milliseconds of silence, hence the sample appears to be missing
    // in the ATI patch.
    // This patch extracts the correct 32 bit length from the RIFF data directly,
    // which fixes this bug.
    patch(m_ub ? 0x419ED8 : 0x419FC8 ,"66 8B 7B 04", "8B 7E FC 90");

    // Pass raw pan values to the sound functions to maintain full precision.
    std::string panPatchOriginal = "C1 F8 08 05 80 00 00 00";
    std::string panPatchReplace  = "90 90 90 90 90 90 90 90";
    patch(m_ub ? 0x4385DF : 0x438C1F, panPatchOriginal, panPatchReplace);
    patch(m_ub ? 0x438631 : 0x438C71, panPatchOriginal, panPatchReplace);
    patch(m_ub ? 0x4386E0 : 0x438D20, panPatchOriginal, panPatchReplace);

    // The ATI patch lacks support for looping sounds. This patch finishes the
    // undone work and replaces the sound loop function stubs with actual
    // implementations.
    // It also replaces the subroutine for normal sounds to fix the annoying
    // panning issue.
    TombRaiderHooks::m_tombSoundInit = reinterpret_cast<TombRaiderSoundInit*>(m_ub ? 0x419DA0 : 0x419E90);
    TombRaiderHooks::m_tombSampleTable = reinterpret_cast<TombRaiderAudioSample***>(m_ub ? 0x45B314 : 0x45B954);
    TombRaiderHooks::m_tombSoundInit1 = reinterpret_cast<BOOL*>(m_ub ? 0x459CF4 : 0x45A31C);
    TombRaiderHooks::m_tombSoundInit2 = reinterpret_cast<BOOL*>(m_ub ? 0x459CF8 : 0x45A320);
    TombRaiderHooks::m_tombDecibelLut = reinterpret_cast<int32_t*>(m_ub ? 0x45E9E0 : 0x45F1E0);

    if (m_ub) {
        patchAddr(0x437B59, "E8 42 22 FE FF", TombRaiderHooks::soundInit, 0xE8);
        patchAddr(0x4386CA, "E8 01 18 FF FF", TombRaiderHooks::setVolume, 0xE8);
        patchAddr(0x4386EA, "E8 E1 17 FF FF", TombRaiderHooks::setPan, 0xE8);
        patchAddr(0x4385F2, "E8 29 F2 FF FF", TombRaiderHooks::playOneShot, 0xE8);
        patchAddr(0x438648, "E8 A3 F2 FF FF", TombRaiderHooks::playLoop, 0xE8);
        patchAddr(0x42EAF8, "E8 F3 8D 00 00", TombRaiderHooks::playLoop, 0xE8);
    } else {
        patchAddr(0x438129, "E8 62 1D FE FF", TombRaiderHooks::soundInit, 0xE8);
        patchAddr(0x438D0A, "E8 21 F2 FF FF", TombRaiderHooks::setVolume, 0xE8);
        patchAddr(0x438D2A, "E8 01 F2 FF FF", TombRaiderHooks::setPan, 0xE8);
        patchAddr(0x438C32, "E8 D9 F1 FF FF", TombRaiderHooks::playOneShot, 0xE8);
        patchAddr(0x438C88, "E8 33 EF FF FF", TombRaiderHooks::playLoop, 0xE8);
        patchAddr(0x42EF35, "E8 86 8C 00 00", TombRaiderHooks::playLoop, 0xE8);
    }

    // Very optional patch: replace ambient track "derelict" with "water", which,
    // in my very personal opinion, is more fitting for the theme of this level.
    if (!m_ub && m_config.getBool("patch_lostvalley_ambience", false)) {
        patch(0x456A1E, "39", "3A");
    }

    // CD audio patches.
    TombRaiderHooks::m_tombMciDeviceID = reinterpret_cast<MCIDEVICEID*>(m_ub ? 0x45B344 : 0x45B994);
    TombRaiderHooks::m_tombAuxDeviceID = reinterpret_cast<uint32_t*>(m_ub ? 0x45B338 : 0x45B984);
    TombRaiderHooks::m_tombHwnd = reinterpret_cast<HWND*>(m_ub ? 0x462E00 : 0x463600);
    TombRaiderHooks::m_tombCDTrackID = reinterpret_cast<int32_t*>(m_ub ? 0x4534F4 : 0x4534DC);
    TombRaiderHooks::m_tombCDTrackIDLoop = reinterpret_cast<int32_t*>(m_ub ? 0x45B330 : 0x45B97C);
    TombRaiderHooks::m_tombCDLoop = reinterpret_cast<BOOL*>(m_ub ? 0x45B30C : 0x45B94C);
    TombRaiderHooks::m_tombCDVolume = reinterpret_cast<uint32_t*>(m_ub ? 0x455D3C : 0x456334);

    // Patch bad mapping function in UB that remaps the music volume from 0-10 to
    // 5-255 instead of 0-65536, which is the value range for auxSetVolume.
    if (m_ub) {
        patchAddr(0x438A70, "0F BF 44 24 04", TombRaiderHooks::updateCDVolume, 0xE9);
    }

    // Hook low-level CD play function to fix a volume bug.
    patchAddr(m_ub ? 0x4379E0 : 0x437FB0, "83 EC 18 53 8B", TombRaiderHooks::playCD, 0xE9);

    // Soundtrack patch. Allows both ambient and music cues to be played via MCI.
    if (!m_ub && m_config.getBool("patch_soundtrack", false)) {
        // hook play function (level music)
        patchAddr(0x438D40, "66 83 3D 34 63", TombRaiderHooks::playCDRemap, 0xE9);
        // hook play function (cutscene music)
        patchAddr(0x439030, "66 83 3D 34 63", TombRaiderHooks::playCDRemap, 0xE9);
        // hook stop function
        patchAddr(0x438E40, "66 A1 DC 34 45", TombRaiderHooks::stopCD, 0xE9);
        // hook function that is called when a track has finished
        patchAddr(0x4380C0, "A1 4C B9 45 00", TombRaiderHooks::playCDLoop, 0xE9);

        // also pass 0 to the CD play sub when loading a level so the background
        // track can be silenced correctly
        patch(0x43639E, "74 09", "90 90");
    }
}

void TombRaiderPatcher::applyLogicPatches() {
    // This changes the first drive letter to search for the Tomb Raider CD from 'C'
    // to 'A', which allows the game to find CDs placed in the drives A: or B: in
    // systems with no floppy drives.
    patch(m_ub ? 0x41BF50 : 0x41C020, "B0 43", "B0 41");

    // This patch fixes a bug in the global key press handling, which normally
    // interrupts the demo mode and the credit sceens immediately after any key
    // has ever been pressed while the game is running.
    TombRaiderHooks::m_tombKeyStates = reinterpret_cast<uint8_t**>(m_ub ? 0x45B348 : 0x45B998);
    TombRaiderHooks::m_tombDefaultKeyBindings = reinterpret_cast<int16_t*>(m_ub ? 0x454880 : 0x454A08);
    TombRaiderHooks::m_tombHhk = reinterpret_cast<HHOOK*>(m_ub ? 0x45A314 : 0x45A93C);

    // replace keyboard hook
    std::vector<uint8_t> tmp;
    appendBytes(reinterpret_cast<int32_t>(TombRaiderHooks::keyboardProc), tmp);
    if (m_ub)  {
        patch(0x43D518, "C0 D1 43 00", tmp);
    } else {
        patch(0x43DC30, "C0 D8 43 00", tmp);
    }

    // hook keypress subroutine
    if (m_ub) {
        patchAddr(0x41E0E0, "8B 54 24 04 8B", TombRaiderHooks::keyIsPressed, 0xE9);
    } else {
        patchAddr(0x41E3E0, "8B 4C 24 04 56", TombRaiderHooks::keyIsPressed, 0xE9);
    }

    // disable internal scan code remapping
    patch(m_ub ? 0x42EC81 : 0x42F151, "75 0A", "EB 34");

    // Fix infinite loop before starting the credits.
    patch(m_ub ? 0x41CC88 : 0x41CD58, "74", "EB");

    // Fix black frames in between the credit screens.
    if (m_ub) {
        patch(0x41D1F3, "D9 CC 00 00", "57 BC FE FF");
        patch(0x41D226, "A6 CC 00 00", "24 BC FE FF");
        patch(0x41D259, "73 CC 00 00", "F1 BB FE FF");
        patch(0x41D28C, "40 CC 00 00", "BE BB FE FF");
        patch(0x41D2BF, "0D CC 00 00", "8B BB FE FF");
    } else {
        patch(0x41D48F, "9D AA 01 00", "AE B9 FE FF");
        patch(0x41D4C2, "6A AA 01 00", "7B B9 FE FF");
        patch(0x41D4F5, "37 AA 01 00", "48 B9 FE FF");
        patch(0x41D528, "04 AA 01 00", "15 B9 FE FF");
    }

    // No-CD patch. Allows the game to load game files and movies from the local
    // directory instead from the CD.
    if (m_config.getBool("patch_nocd", false)) {
        // disable CD check call
        if (m_ub) {
            patch(0x41DE7F, "E8 CC E0 FF FF", "90 90 90 90 90");
        } else {
            patch(0x41E17F, "E8 9C DE FF FF", "90 90 90 90 90");
        }

        // fix format string: "%c:\%s" -> "%s"
        patch(m_ub ? 0x453730 : 0x453890, "25 63 3A 5C 25 73", "25 73 00 00 00 00");

        if (m_ub) {
            // disable drive letter argument in sprintf call
            patch(0x41BF15, "50", "90");
            patch(0x41BF35, "51", "90");
            patch(0x41BF47, "10", "0C");

            patch(0x41AEFC, "50", "90");
            patch(0x41AF0B, "51", "90");
            patch(0x41AF1D, "10", "0C");
        } else {
            // swap drive letter and path in sprintf call
            patch(0x41BFF9, "52 50", "50 52");
            patch(0x41AFE1, "52 50", "50 52");
        }
    }

    // Because the savegame format is incompatible to the DOS version or other
    // patches, change savegame name format from "savegame.%d" to "saveati.%d"
    // to prevent accidental savegame corruption if multiple tomb.exe versions
    // share the same game folder.
    // UB already uses "saveuba.%d", so it doesn't need to be patched.
    if (!m_ub) {
        patch(0x453CCC, "67 61 6D 65 2E 25 64 00", "61 74 69 2E 25 64 00 00");
    }

    // Random fun patches, discovered from various experiments.
    
    // Crazy/creepy SFX mod. Forces a normally unused raw reading mode on all 
    // level samples. The result is hard to describe, just try it out and listen.
    // (requires sample length patch above to be disabled)
    //patch(0x437D1C, "75", "EB");

    // This forces all sounds to be played globally with full volume regardless
    // of their distance to Lara. Can be useful for sound debugging.
    //patch(0x42AAC6, "75 15", "90 90");

    // Underwater mod. Render everything as if it was beneath water. Trippy!
    //patch(0x417216, "26 94", "C6 93");
    //patch(0x416E08, "34 98", "D4 97");

    // Swap Bacon Lara and normal Lara models. Works correctly in Atlantis levels
    // only, but gives funny results otherwise.
    //patch(0x436477, "80 86 42", "A0 7A 41");
    //patch(0x416E17, "85 0C 00", "65 18 01");
}

}