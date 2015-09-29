#include "TombRaiderHooks.hpp"

#include "Logger.hpp"

#include <cmath>

namespace glrage {

bool TombRaiderHooks::m_ub = false;

/** Tomb Raider sub pointers **/

// init sound system
TombRaiderSoundInit* TombRaiderHooks::m_tombSoundInit = nullptr;

/** Tomb Raider var pointers **/

// Pointer to the key state table. If an entry is 1, then the key is pressed.
uint8_t** TombRaiderHooks::m_tombKeyStates = nullptr;

// Default key binding table. Maps the key scan codes to button IDs.
int16_t* TombRaiderHooks::m_tombDefaultKeyBindings = nullptr;

// Audio sample pointer table.
TombRaiderAudioSample*** TombRaiderHooks::m_tombSampleTable = nullptr;

// Sound init booleans. There are two for some reason and both are set to 1 at
// the same location.
BOOL* TombRaiderHooks::m_tombSoundInit1 = nullptr;
BOOL* TombRaiderHooks::m_tombSoundInit2 = nullptr;

// Linear to logarithmic lookup table for decibel conversion.
int32_t* TombRaiderHooks::m_tombDecibelLut = nullptr;

// CD track currently played.
int32_t* TombRaiderHooks::m_tombCDTrackID = nullptr;

// CD track to play after the current one has finished. Usually for ambiance tracks.
int32_t* TombRaiderHooks::m_tombCDTrackIDLoop = nullptr;

// CD loop flag. If TRUE, it re-plays m_tombCDTrackIDLoop after it finished.
BOOL* TombRaiderHooks::m_tombCDLoop = nullptr;

// Current music volume, ranging from 0 to 10.
uint32_t* TombRaiderHooks::m_tombCDVolume = nullptr;

// MCI device ID.
MCIDEVICEID* TombRaiderHooks::m_tombMciDeviceID = nullptr;

// Auxiliary device ID.
uint32_t* TombRaiderHooks::m_tombAuxDeviceID = nullptr;

// Window handle.
HWND* TombRaiderHooks::m_tombHwnd = nullptr;

// Keyboard hook handle.
HHOOK* TombRaiderHooks::m_tombHhk = nullptr;

int32_t TombRaiderHooks::soundInit() {
    int32_t result = m_tombSoundInit();

    // Improves accuracy of the dB LUT, which is used to convert the volume to
    // hundredths of decibels (or, you know, thousandths of bels) for DirectSound
    // The difference is probably barely audible, if at all, but it's still an
    // improvement.
    for (int i = 1; i < DECIBEL_LUT_SIZE; i++) {
        // original formula, 1 dB steps
        //m_tombDecibelLut[i] = 100 * static_cast<int32_t>(-90.0 - std::log2(1.0 / i) * -10.0 / std::log2(0.5));
        // new formula, full dB precision
        m_tombDecibelLut[i] = static_cast<int32_t>(-9000.0 - std::log2(1.0 / i) * -1000.0 / std::log2(0.5));
    }

    return result;
}

int32_t TombRaiderHooks::convertVolumeToDecibel(int32_t volume) {
    // convert volume to dB using the lookup table
    return m_tombDecibelLut[(volume & 0x7FFF) >> 6];
}

int32_t TombRaiderHooks::convertPanToDecibel(uint16_t pan) {
    // Use the panning as the rotation ranging from 0 to 0xffff, convert it to
    // radians, apply the sine function and convert it to a value in the rage
    // -256 to 256 for the volume LUT.
    // Note that I used "DB_CONV_LUT_SIZE / 2" here, which limits the attenuation
    // for one channel to -50 dB to prevent it from being completely silent.
    // This is a workaround for sounds that are played directly at Lara's
    // position, which often have incorrect pannings and flip more or less
    // randomly between the channels.
    int32_t result = static_cast<int32_t>(std::sin((pan / 32767.0) * M_PI) * (DECIBEL_LUT_SIZE / 2));

    if (result > 0) {
        result = -m_tombDecibelLut[DECIBEL_LUT_SIZE - result];
    } else if (result < 0) {
        result = m_tombDecibelLut[DECIBEL_LUT_SIZE + result];
    } else {
        result = 0;
    }

    return result;
}

LPDIRECTSOUNDBUFFER TombRaiderHooks::playSample(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, bool loop) {
    // check if sound system is initialized
    if (!*m_tombSoundInit1 || !*m_tombSoundInit2) {
        return nullptr;
    }

    TombRaiderAudioSample* sample = (*m_tombSampleTable)[soundID];

    // check if sample is valid
    if (!sample) {
        return nullptr;
    }

    LPDIRECTSOUNDBUFFER buffer = sample->buffer;

    // calculate pitch from sample rate
    int32_t dsPitch = sample->sampleRate;

    if (pitch != -1) {
        dsPitch = dsPitch * pitch / 100;
    }

    buffer->SetFrequency(dsPitch);
    buffer->SetPan(convertPanToDecibel(pan));
    buffer->SetVolume(convertVolumeToDecibel(volume));
    buffer->SetCurrentPosition(0);
    buffer->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);

    return buffer;
}

LPDIRECTSOUNDBUFFER TombRaiderHooks::playOneShot(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan) {
    return playSample(soundID, volume, pitch, pan, false);
}

LPDIRECTSOUNDBUFFER TombRaiderHooks::playLoop(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, int32_t a5, int32_t a6, int32_t a7) {
    return playSample(soundID, volume, pitch, pan, true);
}

LRESULT TombRaiderHooks::keyboardProc(int32_t nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        goto next;
    }

    uint32_t keyData = static_cast<uint32_t>(lParam);
    uint32_t scanCode = keyData >> 16 & 0xff;
    uint32_t extended = keyData >> 24 & 0x1;
    uint32_t pressed = ~keyData >> 31;
    
    uint32_t indexCode = scanCode;

    // Remap DOS scan code for certain control keys (ALT, CTRL and SHIFT).
    // This is also done in the original code, but at several different places
    // and also in reverse, which may be responsible for the stuck keys bug.
    // It's an ugly hack but still better than patching both the default key
    // mappings and the key names.
    if (indexCode == 29) {
        indexCode = 157;
    }

    if (indexCode == 42) {
        indexCode = 54;
    }

    if (indexCode == 56) {
        indexCode = 184;
    }

    if (extended) {
        indexCode += 128;
    }

    uint8_t* keyStates = *m_tombKeyStates;
    if (!keyStates) {
        goto next;
    }

    keyStates[indexCode] = pressed;
    keyStates[325] = pressed;

    // terminate ALT key hook so the menu won't pop up when jumping
    if (wParam == VK_MENU) {
        return 1;
    }

    next:
    return CallNextHookEx(*m_tombHhk, nCode, wParam, lParam);
}

BOOL TombRaiderHooks::keyIsPressed(int32_t keyCode) {
    int16_t keyBinding = m_tombDefaultKeyBindings[keyCode];
    uint8_t* keyStates = *m_tombKeyStates;
    return keyStates[keyBinding];
}

BOOL TombRaiderHooks::playCDRemap(int16_t trackID) {
    LOGF("playCDRemap(%d)", trackID);
    
    // stop CD play on track ID 0
    if (trackID == 0) {
        stopCD();
        return FALSE;
    }

    // ignore redundant PSX ambience track (replaced by 57)
    if (trackID == 5) {
        return FALSE;
    }

    // set current track ID
    *m_tombCDTrackID = trackID;

    return playCD(trackID);
}

BOOL TombRaiderHooks::playCDLoop() {
    LOG("playCDLoop()");

    // cancel if there's currently no looping track set
    if (*m_tombCDLoop && *m_tombCDTrackIDLoop > 0) {
        playCD(*m_tombCDTrackIDLoop);
        return FALSE;
    }

    return *m_tombCDLoop;
}

BOOL TombRaiderHooks::playCD(int16_t trackID) {
    LOGF("playCD(%d)", trackID);

    // don't play music tracks if volume is set to 0
    if (!*m_tombCDVolume) {
        return FALSE;
    }

    // don't try to play data track
    if (trackID < 2) {
        return FALSE;
    }

    // set looping track ID for ambience tracks
    if (m_ub || trackID >= 57) {
        *m_tombCDTrackIDLoop = trackID;
    }

    // this one will always be set back to true on the next tick
    *m_tombCDLoop = FALSE;

    // Calculate volume from current volume setting. The original code used
    // the hardcoded value 0x400400, which equals a volume of 25%.
    uint32_t volume = *m_tombCDVolume * 0xffff / 10;
    volume |= volume << 16;
    auxSetVolume(*m_tombAuxDeviceID, volume);

    // configure player
    MCI_SET_PARMS setParms;
    setParms.dwTimeFormat = MCI_FORMAT_TMSF;
    if (mciSendCommand(*m_tombMciDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
        reinterpret_cast<DWORD_PTR>(&setParms))) {
        return FALSE;
    }

    // get length of track to determine dwTo
    MCI_STATUS_PARMS statusParms;
    statusParms.dwItem = MCI_STATUS_LENGTH;
    statusParms.dwTrack = trackID;
    if (mciSendCommand(*m_tombMciDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK,
        reinterpret_cast<DWORD_PTR>(&statusParms))) {
        return FALSE;
    }

    // send play command
    MCI_PLAY_PARMS openParms;
    openParms.dwCallback = reinterpret_cast<DWORD_PTR>(*m_tombHwnd);
    openParms.dwFrom = trackID;
    openParms.dwTo = statusParms.dwReturn;
    if (mciSendCommand(*m_tombMciDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM | MCI_TO,
        reinterpret_cast<DWORD_PTR>(&openParms))) {
        return FALSE;
    }

    return TRUE;
}

BOOL TombRaiderHooks::stopCD() {
    LOG("stopCD()");

    *m_tombCDTrackID = 0;
    *m_tombCDTrackIDLoop = 0;
    *m_tombCDLoop = FALSE;

    // The original code used MCI_PAUSE, probably to reduce latency when switching
    // tracks. But we'll use MCI_STOP here, since it's expected to use an MCI
    // wrapper with nearly zero latency anyway.
    MCI_GENERIC_PARMS genParms;
    return !mciSendCommand(*m_tombMciDeviceID, MCI_STOP, MCI_WAIT,
        reinterpret_cast<DWORD_PTR>(&genParms));
}

BOOL TombRaiderHooks::updateCDVolume(int16_t volume) {
    LOGF("updateCDVolume(%d)", volume);

    uint32_t volumeAux = volume * 0xffff / 0xff;
    volumeAux |= volumeAux << 16;
    auxSetVolume(*m_tombAuxDeviceID, volumeAux);

    return TRUE;
}

void TombRaiderHooks::setVolume(LPDIRECTSOUNDBUFFER buffer, int32_t volume) {
    if (buffer) {
        buffer->SetVolume(convertVolumeToDecibel(volume));
    }
}

void TombRaiderHooks::setPan(LPDIRECTSOUNDBUFFER buffer, int32_t pan) {
    if (buffer) {
        buffer->SetPan(convertPanToDecibel(pan));
    }
}

}