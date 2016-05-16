#include "TombRaiderPatcher.hpp"
#include "TombRaiderHooks.hpp"

#include <glrage/GLRage.hpp>
#include <glrage_util/ErrorUtils.hpp>
#include <glrage_util/Logger.hpp>
#include <glrage_util/StringUtils.hpp>

#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace glrage {

TombRaiderPatcher::TombRaiderPatcher(bool ub)
    : m_ub(ub)
{
}

GameID TombRaiderPatcher::gameID()
{
    return m_ub ? GameID::TombRaiderGold : GameID::TombRaider;
}

void TombRaiderPatcher::apply()
{
    // mandatory crash patches
    applyCrashPatches();

    // optional patches
    applyGraphicPatches();
    applySoundPatches();
    applyLogicPatches();
}

void TombRaiderPatcher::applyCrashPatches()
{
    // Tomb Raider ATI patch fails on later Windows versions because of a
    // missing return statement in a function.
    // In Windows 95, it is compensated by OutputDebugString's nonzero eax
    // value, while in later Windows versions, OutputDebugString returns zero.
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
    // damage to Lara, but that's still better than no explosions or even a
    // crash.
    patch(m_ub ? 0x43C288 : 0x43C938, "F6 C3 1C 74", "90 90 90 EB");

    // Fix for a crash when opening the control options in-game while the FPS
    // counter is visible. There are apparently too many text overlays visible
    // in that case and there's no error handling when the text overlay creation
    // fails, so a positioning function is fed with a null pointer when
    // rendering the "Inventory" label. This patch prevents the text overlay sub
    // from creating a null pointer if there are too many text overlays.
    patch(m_ub ? 0x4390E3 : 0x439793, "33 C0", "90 90");
}

void TombRaiderPatcher::applyGraphicPatches()
{
    // The ATI version of Tomb Raider converts vertex colors to half of the
    // original brightness, which results in a dim look and turns some areas in
    // dark levels almost pitch black. This patch boosts the brightness back to
    // normal levels.
    if (m_ctx.config.getBool("brightness_override", true)) {
        float brightness = m_ctx.config.getFloat("brightness_value", 1.0f);
        float divisor = (1.0f / brightness) * 1024;
        float multi = 0.0625f * brightness;

        m_tmp.clear();
        m_tmp << divisor;

        patch(0x451034, "00 00 00 45", m_tmp);

        m_tmp.clear();
        m_tmp << multi;

        patch(0x45103C, "DB F6 FE 3C", m_tmp);
    }

    // This patch allows the customization of the water color, which is rather
    // ugly on default.
    if (m_ctx.config.getBool("watercolor_override", true)) {
        float filterRed =
            m_ctx.config.getFloat("watercolor_filter_red", 0.45f);
        float filterGreen =
            m_ctx.config.getFloat("watercolor_filter_green", 1.0f);

        m_tmp.clear();
        m_tmp << filterRed << filterGreen;

        patch(0x451028, "9A 99 19 3F 33 33 33 3F", m_tmp);
    }

    // This patch replaces 800x600 with a custom resolution for widescreen
    // support and to reduce vertex artifacts due to subpixel inaccuracy.
    if (m_ctx.config.getBool("resolution_override", true)) {
        int32_t width = m_ctx.config.getInt("resolution_width", -1);
        int32_t height = m_ctx.config.getInt("resolution_height", -1);

        if (width <= 0) {
            width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        }

        if (height <= 0) {
            height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        }

        // update display mode and viewport parameters
        m_tmp.clear();
        m_tmp << width;
        patch(m_ub ? 0x407CAA : 0x407C9D, "20 03 00 00", m_tmp);

        m_tmp.clear();
        m_tmp << height;
        patch(m_ub ? 0x407CB4 : 0x407CA7, "58 02 00 00", m_tmp);

        m_tmp.clear();
        m_tmp << static_cast<float_t>(width - 1);
        patch(m_ub ? 0x407CBE : 0x407CB1, "00 C0 47 44", m_tmp);

        m_tmp.clear();
        m_tmp << static_cast<float_t>(height - 1);
        patch(m_ub ? 0x407CC8 : 0x407CBB, "00 C0 15 44", m_tmp);

        // update clipping size
        m_tmp.clear();
        m_tmp << static_cast<int16_t>(width);
        patch(m_ub ? 0x408A64 : 0x408A57, "20 03", m_tmp);

        m_tmp.clear();
        m_tmp << static_cast<int16_t>(height);
        patch(m_ub ? 0x408A6D : 0x408A60, "58 02", m_tmp);

        // set display string (needs to be static so the data won't vanish after
        // patching has finished)
        static std::string displayMode =
            StringUtils::format("%dx%d", 24, width, height);

        m_tmp.clear();
        m_tmp << reinterpret_cast<int32_t>(displayMode.c_str());
        if (m_ub) {
            patch(0x42DB5B, "40 61 45 00 ", m_tmp);
        } else {
            patch(0x42DF6B, "58 67 45 00", m_tmp);
        }

        // UI scale patch, rescales the in-game overlay to keep the proportions
        // of the 800x600 resolution on higher resolutions.
        TombRaiderHooks::m_tombRenderLine =
            reinterpret_cast<TombRaiderRenderLine*>(0x402710);
        TombRaiderHooks::m_tombRenderCollectedItem =
            reinterpret_cast<TombRaiderRenderCollectedItem*>(
                m_ub ? 0x435800 : 0x435D80);
        TombRaiderHooks::m_tombCreateOverlayText =
            reinterpret_cast<TombRaiderCreateOverlayText*>(
                m_ub ? 0x4390D0 : 0x439780);
        TombRaiderHooks::m_tombRenderWidth =
            reinterpret_cast<int32_t*>(m_ub ? 0x6CA5D4 : 0x6CADD4);
        TombRaiderHooks::m_tombRenderHeight =
            reinterpret_cast<int32_t*>(m_ub ? 0x68EBA8 : 0x68F3A8);
        TombRaiderHooks::m_tombTicks =
            reinterpret_cast<int32_t*>(m_ub ? 0x459CF0 : 0x45A318);

        // clang-format off
        if (m_ub) {
            patchAddr(0x41DA85, "E8 76 23 01 00", TombRaiderHooks::renderHealthBar, 0xE8);
            patchAddr(0x41DC0C, "E8 EF 21 01 00", TombRaiderHooks::renderHealthBar, 0xE8);
            patchAddr(0x41DAD7, "E8 A4 24 01 00", TombRaiderHooks::renderAirBar, 0xE8);
            patchAddr(0x41DB71, "E8 8A 7C 01 00", TombRaiderHooks::renderCollectedItem, 0xE8);
            patchAddr(0x4394C2, "E8 09 FC FF FF", TombRaiderHooks::createFPSText, 0xE8);
        } else {
            patchAddr(0x41DD85, "E8 46 25 01 00", TombRaiderHooks::renderHealthBar, 0xE8);
            patchAddr(0x41DF0C, "E8 BF 23 01 00", TombRaiderHooks::renderHealthBar, 0xE8);
            patchAddr(0x41DDD7, "E8 74 26 01 00", TombRaiderHooks::renderAirBar, 0xE8);
            patchAddr(0x41DE71, "E8 0A 7F 01 00", TombRaiderHooks::renderCollectedItem, 0xE8);
            patchAddr(0x439B72, "E8 09 FC FF FF", TombRaiderHooks::createFPSText, 0xE8);
        }
        // clang-format on
    }

    // Field of view customization patch.
    if (m_ctx.config.getBool("fov_override", true)) {
        int32_t fov = m_ctx.config.getInt("fov_value", 65);

        int8_t fov8 = static_cast<int8_t>(fov);
        m_tmp.clear();
        m_tmp << fov8;

        patch(m_ub ? 0x4163E2 : 0x4164D2, "50", m_tmp);

        int16_t fov16 = fov * 182;
        m_tmp.clear();
        m_tmp << fov16;

        patch(m_ub ? 0x41AAAA : 0x41AB9A, "E0 38", m_tmp);
        patch(m_ub ? 0x41E45B : 0x41E7DB, "E0 38", m_tmp);

        // change the FOV mode from horizontal to vertical if enabled
        if (m_ctx.config.getBool("fov_vertical", true)) {
            TombRaiderHooks::m_tombSetFOV =
                reinterpret_cast<TombRaiderSetFOV*>(0x4026D0);

            // replace inline FOV conversion code with function call that
            // contains the same code
            if (m_ub) {
                patch(0x402666, "56 E8 64 7D 02 00 8B F8 A1 D4",
                    "D1 E6 56 E8 62 00 00 00 EB 1B");
            } else {
                patch(0x402666, "56 E8 84 81 02 00 8B F8 A1 D4",
                    "D1 E6 56 E8 62 00 00 00 EB 1B");
            }

            // replace FOV conversion function with custom function
            patchAddr(
                0x402669, "E8 62 00 00 00", TombRaiderHooks::setFOV, 0xE8);

            // clang-format off
            if (m_ub) {
                patchAddr(0x4113E9, "E8 E2 12 FF FF", TombRaiderHooks::setFOV, 0xE8);
                patchAddr(0x411624, "E8 A7 10 FF FF", TombRaiderHooks::setFOV, 0xE8);
                patchAddr(0x41AACC, "E8 FF 7B FE FF", TombRaiderHooks::setFOV, 0xE8);
                patchAddr(0x41E45F, "E8 6C 42 FE FF", TombRaiderHooks::setFOV, 0xE8);
            } else {
                patchAddr(0x411469, "E8 62 12 FF FF", TombRaiderHooks::setFOV, 0xE8);
                patchAddr(0x41171A, "E8 B1 0F FF FF", TombRaiderHooks::setFOV, 0xE8);
                patchAddr(0x41ABBC, "E8 0F 7B FE FF", TombRaiderHooks::setFOV, 0xE8);
                patchAddr(0x41E7DF, "E8 EC 3E FE FF", TombRaiderHooks::setFOV, 0xE8);
            }
            // clang-format on
        }
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

    if (m_ctx.config.getBool("draw_distance_override", false)) {
        int32_t drawDistFade =
            m_ctx.config.getInt("draw_distance_fade", 12288);
        int32_t drawDistMax = m_ctx.config.getInt("draw_distance_max", 20480);

        RuntimeData drawDistFadeData;
        drawDistFadeData << drawDistFade;

        RuntimeData drawDistFadeNegData;
        drawDistFadeNegData << -drawDistFade;

        RuntimeData drawDistMaxData;
        drawDistMaxData << drawDistMax;

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
    // Disabed: while this works fine in the menu, the game logic runs at 30
    // ticks
    // per second and offers no interpolation, so it's impossible to render more
    // frames without either rendering duplicate frames or speeding up the game
    // time.
    // if (m_ctx.config.getBool("60fps", true)) {
    //    // render on every tick instead of every other
    //    patch(m_ub ? 0x408A91 : 0x408A84, "02", "00");
    //    // disables frame skipping, which also fixes issues with the demo mode
    //    // if the frame rate isn't exactly at limit all the time
    //    patch(m_ub ? 0x408ABA : 0x408AAD, "83 E1 1F", "33 C9 90");
    //}
}

void TombRaiderPatcher::applySoundPatches()
{
    // For reasons unknown, the length of a sound sample is stored in a struct
    // field as a 16 bit integer, which means that the maximum allowed length
    // for a sound sample is be 65535 bytes. If a sample is larger, which
    // happens quite often for Lara's speeches in her home, then there's an
    // integer overflow and the length is wrapped to the modulo of 65535.
    // This causes her speech to cut off, if too long.
    // In one case ("Ah, the main hall..."), the sample is just slightly larger
    // than 64k, which causes the game to play only the first few milliseconds
    // of silence, hence the sample appears to be missing in the ATI patch.
    // This patch extracts the correct 32 bit length from the RIFF data
    // directly, which fixes this bug.
    patch(m_ub ? 0x419ED8 : 0x419FC8, "66 8B 7B 04", "8B 7E FC 90");

    // Pass raw pan values to the sound functions to maintain full precision.
    std::string panPatchOriginal = "C1 F8 08 05 80 00 00 00";
    std::string panPatchReplace = "90 90 90 90 90 90 90 90";
    patch(m_ub ? 0x4385DF : 0x438C1F, panPatchOriginal, panPatchReplace);
    patch(m_ub ? 0x438631 : 0x438C71, panPatchOriginal, panPatchReplace);
    patch(m_ub ? 0x4386E0 : 0x438D20, panPatchOriginal, panPatchReplace);

    // The ATI patch lacks support for looping sounds. This patch finishes the
    // undone work and replaces the sound loop function stubs with actual
    // implementations.
    // It also replaces the subroutine for normal sounds to fix the annoying
    // panning issue.
    TombRaiderHooks::m_tombSoundInit =
        reinterpret_cast<TombRaiderSoundInit*>(m_ub ? 0x419DA0 : 0x419E90);
    TombRaiderHooks::m_tombNumAudioSamples =
        reinterpret_cast<int32_t*>(m_ub ? 0x45B324 : 0x45B96C);
    TombRaiderHooks::m_tombSampleTable =
        reinterpret_cast<TombRaiderAudioSample***>(m_ub ? 0x45B314 : 0x45B954);
    TombRaiderHooks::m_tombSoundInit1 =
        reinterpret_cast<BOOL*>(m_ub ? 0x459CF4 : 0x45A31C);
    TombRaiderHooks::m_tombSoundInit2 =
        reinterpret_cast<BOOL*>(m_ub ? 0x459CF8 : 0x45A320);
    TombRaiderHooks::m_tombDecibelLut =
        reinterpret_cast<int32_t*>(m_ub ? 0x45E9E0 : 0x45F1E0);

    // clang-format off
    if (m_ub) {
        patchAddr(0x437B59, "E8 42 22 FE FF", TombRaiderHooks::soundInit, 0xE8);
        patchAddr(0x4386CA, "E8 01 18 FF FF", TombRaiderHooks::soundSetVolume, 0xE8);
        patchAddr(0x4386EA, "E8 E1 17 FF FF", TombRaiderHooks::soundSetPan, 0xE8);
        patchAddr(0x4385F2, "E8 29 F2 FF FF", TombRaiderHooks::soundPlayOneShot, 0xE8);
        patchAddr(0x438648, "E8 A3 F2 FF FF", TombRaiderHooks::soundPlayLoop, 0xE8);
        patchAddr(0x438680, "A1 74 60 45 00", TombRaiderHooks::soundStopAll, 0xE9);
    } else {
        patchAddr(0x438129, "E8 62 1D FE FF", TombRaiderHooks::soundInit, 0xE8);
        patchAddr(0x438D0A, "E8 21 F2 FF FF", TombRaiderHooks::soundSetVolume, 0xE8);
        patchAddr(0x438D2A, "E8 01 F2 FF FF", TombRaiderHooks::soundSetPan, 0xE8);
        patchAddr(0x438C32, "E8 D9 F1 FF FF", TombRaiderHooks::soundPlayOneShot, 0xE8);
        patchAddr(0x438C88, "E8 33 EF FF FF", TombRaiderHooks::soundPlayLoop, 0xE8);
        patchAddr(0x438CC0, "A1 88 66 45 00", TombRaiderHooks::soundStopAll, 0xE9);
    }
    // clang-format on

    // Very optional patch: change ambient track in Lost Valley from "derelict"
    // to "water", which, in my very personal opinion, is more fitting for the
    // theme of this level.
    if (!m_ub && m_ctx.config.getBool("lostvalley_ambience", false)) {
        patch(0x456A1E, "39", "3A");
    }

    // CD audio patches.
    TombRaiderHooks::m_tombMciDeviceID =
        reinterpret_cast<MCIDEVICEID*>(m_ub ? 0x45B344 : 0x45B994);
    TombRaiderHooks::m_tombAuxDeviceID =
        reinterpret_cast<uint32_t*>(m_ub ? 0x45B338 : 0x45B984);
    TombRaiderHooks::m_tombHwnd =
        reinterpret_cast<HWND*>(m_ub ? 0x462E00 : 0x463600);
    TombRaiderHooks::m_tombCDTrackID =
        reinterpret_cast<int32_t*>(m_ub ? 0x4534F4 : 0x4534DC);
    TombRaiderHooks::m_tombCDTrackIDLoop =
        reinterpret_cast<int32_t*>(m_ub ? 0x45B330 : 0x45B97C);
    TombRaiderHooks::m_tombCDLoop =
        reinterpret_cast<BOOL*>(m_ub ? 0x45B30C : 0x45B94C);
    TombRaiderHooks::m_tombCDVolume =
        reinterpret_cast<uint32_t*>(m_ub ? 0x455D3C : 0x456334);
    TombRaiderHooks::m_tombCDNumTracks =
        reinterpret_cast<uint32_t*>(m_ub ? 0x45B31C : 0x45B964);

    // Patch bad mapping function in UB that remaps the music volume from 0-10
    // to 5-255 instead of 0-65536, which is the value range for auxSetVolume.
    if (m_ub) {
        patchAddr(
            0x438A70, "0F BF 44 24 04", TombRaiderHooks::musicSetVolume, 0xE9);
    }

    // Hook low-level CD play function to fix a volume bug.
    patchAddr(m_ub ? 0x4379E0 : 0x437FB0, "83 EC 18 53 8B",
        TombRaiderHooks::musicPlay, 0xE9);

    // Soundtrack patch. Allows both ambient and music cues to be played via
    // MCI.
    if (m_ctx.config.getBool("full_soundtrack", false)) {
        // hook play function (level music)
        if (m_ub) {
            patchAddr(0x438700, "66 83 3D 3C 5D",
                TombRaiderHooks::musicPlayRemap, 0xE9);
        } else {
            patchAddr(0x438D40, "66 83 3D 34 63",
                TombRaiderHooks::musicPlayRemap, 0xE9);
        }

        // hook play function (cutscene music, not present in UB)
        if (!m_ub) {
            patchAddr(0x439030, "66 83 3D 34 63",
                TombRaiderHooks::musicPlayRemap, 0xE9);
        }

        // hook stop function
        if (m_ub) {
            patchAddr(
                0x438880, "66 A1 F4 34 45", TombRaiderHooks::musicStop, 0xE9);
        } else {
            patchAddr(
                0x438E40, "66 A1 DC 34 45", TombRaiderHooks::musicStop, 0xE9);
        }

        // hook function that is called when a track has finished
        if (m_ub) {
            patchAddr(0x437AF0, "A1 0C B3 45 00",
                TombRaiderHooks::musicPlayLoop, 0xE9);
        } else {
            patchAddr(0x4380C0, "A1 4C B9 45 00",
                TombRaiderHooks::musicPlayLoop, 0xE9);
        }

        // also pass 0 to the CD play sub when loading a level so the background
        // track can be silenced correctly
        if (m_ub) {
            patch(0x437AF7, "74 0E", "90 90");
        } else {
            patch(0x43639E, "74 09", "90 90");
        }

        // fix level audio mapping to match the track list of the original game
        if (m_ub) {
            patch(
                0x456400, "03 00 03 00 04 00 04 00", "3B 00 3B 00 3C 00 3C 00");
        }
    } else {
        // without soundtrack, there are only three tracks left for UB,
        // which are all looping
        TombRaiderHooks::m_musicAlwaysLoop = m_ub;
    }
}

void TombRaiderPatcher::applyLogicPatches()
{
    // This changes the first drive letter to search for the Tomb Raider CD from
    // 'C' to 'A', which allows the game to find CDs placed in the drives A: or
    // B: in systems with no floppy drives.
    patch(m_ub ? 0x41BF50 : 0x41C020, "B0 43", "B0 41");

    // This patch fixes a bug in the global key press handling, which normally
    // interrupts the demo mode and the credit sceens immediately after any key
    // has ever been pressed while the game is running.
    TombRaiderHooks::m_tombKeyStates =
        reinterpret_cast<uint8_t**>(m_ub ? 0x45B348 : 0x45B998);
    TombRaiderHooks::m_tombDefaultKeyBindings =
        reinterpret_cast<int16_t*>(m_ub ? 0x454880 : 0x454A08);
    TombRaiderHooks::m_tombHhk =
        reinterpret_cast<HHOOK*>(m_ub ? 0x45A314 : 0x45A93C);

    // replace keyboard hook
    m_tmp.clear();
    m_tmp << reinterpret_cast<int32_t>(TombRaiderHooks::keyboardProc);

    if (m_ub) {
        patch(0x43D518, "C0 D1 43 00", m_tmp);
    } else {
        patch(0x43DC30, "C0 D8 43 00", m_tmp);
    }

    // hook keypress subroutine
    if (m_ub) {
        patchAddr(
            0x41E0E0, "8B 54 24 04 8B", TombRaiderHooks::keyIsPressed, 0xE9);
    } else {
        patchAddr(
            0x41E3E0, "8B 4C 24 04 56", TombRaiderHooks::keyIsPressed, 0xE9);
    }

    // disable internal scan code remapping
    patch(m_ub ? 0x42EC81 : 0x42F151, "75 0A", "EB 34");

    // fix infinite loop before starting the credits
    patch(m_ub ? 0x41CC88 : 0x41CD58, "74", "EB");

    // fix black frames in between the credit screens
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

    // prevent selection of user keys in the options (they don't work anyway)
    patch(m_ub ? 0x42E76C : 0x42EB7C, "0F 94 C0", "90 90 90");

    // No-CD patch. Allows the game to load game files and movies from the local
    // directory instead from the CD.
    if (m_ctx.config.getBool("nocd", false)) {
        // disable CD check call
        if (m_ub) {
            patch(0x41DE7F, "E8 CC E0 FF FF", "90 90 90 90 90");
        } else {
            patch(0x41E17F, "E8 9C DE FF FF", "90 90 90 90 90");
        }

        // fix format string: "%c:\%s" -> "%s"
        patch(m_ub ? 0x453730 : 0x453890, "25 63 3A 5C 25 73",
            "25 73 00 00 00 00");

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

    // Experimental localization patch. Replaces string pointers with pointers
    // for translations.
    if (m_ctx.config.getBool("localization", false)) {
        try {
            applyLocalePatches();
        } catch (const std::runtime_error& ex) {
            // translation files are optional, so simply log if they're missing
            LOG_INFO("Can't apply translation patch: %s", ex.what());
        }
    }

    // Random fun patches, discovered from various experiments.

    // Crazy/creepy SFX mod. Forces a normally unused raw reading mode on all
    // level samples. The result is hard to describe, just try it out and
    // listen.
    // (requires sample length patch above to be disabled)
    // patch(0x437D1C, "75", "EB");

    // This forces all sounds to be played globally with full volume regardless
    // of their distance to Lara. Can be useful for sound debugging.
    // patch(0x42AAC6, "75 15", "90 90");

    // Underwater mod. Render everything as if it was beneath water. Trippy!
    // patch(0x417216, "26 94", "C6 93");
    // patch(0x416E08, "34 98", "D4 97");

    // Swap Bacon Lara and normal Lara models. Works correctly in Atlantis
    // levels only, but gives funny results otherwise.
    // patch(0x436477, "80 86 42", "A0 7A 41");
    // patch(0x416E17, "85 0C 00", "65 18 01");
}

void TombRaiderPatcher::applyLocalePatches()
{
    std::wstring basePath = GLRage::getContext().getBasePath();
    std::wstring localePath = basePath + L"\\patches\\locale\\";

    // load locale file
    std::string locale =
        m_ctx.config.getString("localization_locale", "en_GB");
    std::wstring langPath =
        localePath + StringUtils::utf8ToWide(locale) + L".txt";
    std::ifstream langStream(langPath);

    if (!langStream.good()) {
        throw std::runtime_error(
            StringUtils::format("Can't open translation file '%s': %s", 1024,
                StringUtils::wideToUtf8(langPath),
                ErrorUtils::getSystemErrorString()));
    }

    static std::map<int32_t, std::string> stringMap;

    std::string line;
    for (int32_t lineNum = 0; std::getline(langStream, line); lineNum++) {
        try {
            int32_t stringIndex = std::stoi(line.substr(0, 4));
            stringMap[stringIndex] = line.substr(5, std::string::npos);
        } catch (...) {
            throw std::runtime_error(StringUtils::format(
                "Invalid string index at line %d", 1024, lineNum));
        }
    }

    // load string data file
    std::wstring stringsPath = localePath + L"strings.txt";
    std::ifstream stringsStream(stringsPath);

    if (!stringsStream.good()) {
        throw std::runtime_error(
            StringUtils::format("Can't open translation strings file '%s': %s",
                1024, StringUtils::wideToUtf8(stringsPath),
                ErrorUtils::getSystemErrorString()));
    }

    RuntimeData expected;
    RuntimeData replacement;

    while (std::getline(stringsStream, line)) {
        std::istringstream lineStream(line);

        int32_t stringIndex = 0;
        int32_t stringPos = 0;
        std::string string;
        for (int32_t value, valueIndex = 0; lineStream >> value; valueIndex++) {
            // first value is the index
            if (valueIndex == 0) {
                stringIndex = value;
                if (stringMap.find(stringIndex) == stringMap.end()) {
                    LOG_INFO("Missing translation for %d '%s'", stringIndex,
                        string.c_str());
                    break;
                }

                lineStream >> std::hex;
                continue;
            }

            // second value is the position
            if (valueIndex == 1) {
                stringPos = value;
                expected.clear();
                expected << stringPos;
                string = std::string(reinterpret_cast<char*>(stringPos));
                continue;
            }

            // remaining values are pointers, which need to be patched
            replacement.clear();
            replacement << reinterpret_cast<int32_t>(
                &stringMap[stringIndex][0]);

            patch(value, expected, replacement);
        }
    }
}

} // namespace glrage