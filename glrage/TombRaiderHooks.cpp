#include "TombRaiderHooks.hpp"

#include "Logger.hpp"

#include <cmath>

namespace glrage {

/** Tomb Raider sub pointers **/

// init sound system
TombRaiderSoundInit* TombRaiderHooks::m_tombSoundInit = nullptr;

/** Tomb Raider var pointers **/

// MCI device ID.
MCIDEVICEID* TombRaiderHooks::m_tombMciDeviceID = nullptr;

// Auxiliary device ID.
uint32_t* TombRaiderHooks::m_tombAuxDeviceID = nullptr;

// Window handle.
HWND* TombRaiderHooks::m_tombHwnd = nullptr;

// CD track currently played.
int32_t* TombRaiderHooks::m_tombCDTrackID = nullptr;

// CD track to play after the current one has finished. Usually for ambiance tracks.
int32_t* TombRaiderHooks::m_tombCDTrackIDLoop = nullptr;

bool* TombRaiderHooks::m_tombCDLoop = nullptr;

// Current music volume, ranging from 0 to 10.
uint32_t* TombRaiderHooks::m_tombCDVolume = nullptr;

// Key state table, one byte per key.
uint8_t** TombRaiderHooks::m_tombKeyStates = nullptr;

// Audio sample pointer table.
TombRaiderAudioSample*** TombRaiderHooks::m_tombSampleTable = nullptr;

// Sound init booleans. There are two for some reason and both are set to 1 at
// the same location.
bool* TombRaiderHooks::m_tombSoundInit1 = nullptr;
bool* TombRaiderHooks::m_tombSoundInit2 = nullptr;

// Pointer to linear to log lookup table for decibel conversion.
int32_t* TombRaiderHooks::m_tombDecibelLut = nullptr;

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

void TombRaiderHooks::keyEvent(int32_t extended, int32_t code, int32_t pressed) {
    uint8_t* keyStates = *m_tombKeyStates;
    if (!keyStates) {
        return;
    }

    int codeOffset = code;
    if (extended) {
        codeOffset += 128;
    }

    // 325 is the index for the global key. If any key is pressed, it should
    // also update this index. It is used in "press any key to continue" situations,
    // like the demo mode or the credits.
    keyStates[codeOffset] = pressed;
    keyStates[325] = pressed;

    // Old bugged implementation, which just increments the global key state when
    // any key is pressed, thus leaving the global key in a permanently pressed
    // state util it eventually overflows.
    // The assembly code also uses an unitialized edx register, which may be the
    // reason why the movement keys get sometimes permanenty stuck in a pressed state.
    //if (keyStates[codeOffset] != pressed) {
    //    keyStates[codeOffset] = pressed;
    //    keyStates[325]++;
    //}
}

bool TombRaiderHooks::playCDRemap(int16_t trackID) {
    LOGF("playCDRemap(%d)", trackID);
    
    // stop CD play on track ID 0
    if (trackID == 0) {
        stopCD();
        return false;
    }

    // ignore redundant PSX ambience track (replaced by 57)
    if (trackID == 5) {
        return false;
    }

    // set looping track ID for ambience tracks
    if (trackID >= 57) {
        *m_tombCDTrackIDLoop = trackID;
    }

    // set current track ID
    *m_tombCDTrackID = trackID;

    return playCD(trackID);
}

bool TombRaiderHooks::playCDLoop() {
    LOG("playCDLoop()");

    // cancel if there's currently no looping track set
    if (*m_tombCDLoop && *m_tombCDTrackIDLoop > 0) {
        playCD(*m_tombCDTrackIDLoop);
        return false;
    }

    return *m_tombCDLoop;
}

bool TombRaiderHooks::playCD(int16_t trackID) {
    LOGF("playCD(%d)", trackID);

    // don't play music tracks if volume is set to 0
    if (!*m_tombCDVolume) {
        return false;
    }

    // don't try to play data track
    if (trackID < 2) {
        return false;
    }

    *m_tombCDLoop = false;

    // configure player
    MCI_SET_PARMS setParms;
    setParms.dwTimeFormat = MCI_FORMAT_TMSF;
    if (mciSendCommand(*m_tombMciDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
        reinterpret_cast<DWORD_PTR>(&setParms))) {
        return false;
    }

    // get length of track to determine dwTo
    MCI_STATUS_PARMS statusParms;
    statusParms.dwItem = MCI_STATUS_LENGTH;
    statusParms.dwTrack = trackID;
    if (mciSendCommand(*m_tombMciDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_TRACK,
        reinterpret_cast<DWORD_PTR>(&statusParms))) {
        return false;
    }

    // send play command
    MCI_PLAY_PARMS openParms;
    openParms.dwCallback = reinterpret_cast<DWORD_PTR>(*m_tombHwnd);
    openParms.dwFrom = trackID;
    openParms.dwTo = statusParms.dwReturn;
    if (mciSendCommand(*m_tombMciDeviceID, MCI_PLAY, MCI_NOTIFY | MCI_FROM | MCI_TO,
        reinterpret_cast<DWORD_PTR>(&openParms))) {
        return false;
    }

    // Calculate volume from current volume setting. The original code used
    // the hardcoded value 0x400400, which equals a volume of 25%.
    uint32_t volume = *m_tombCDVolume * 0xffff / 10;
    volume |= volume << 16;
    auxSetVolume(*m_tombAuxDeviceID, volume);

    return true;
}

bool TombRaiderHooks::stopCD() {
    LOG("stopCD()");

    *m_tombCDTrackID = 0;
    *m_tombCDTrackIDLoop = 0;
    *m_tombCDLoop = false;

    // The original code used MCI_PAUSE, probably to reduce latency when switching
    // tracks. But we'll use MCI_STOP here, since it's expected to use an MCI
    // wrapper with nearly zero latency anyway.
    MCI_GENERIC_PARMS genParms;
    return !mciSendCommand(*m_tombMciDeviceID, MCI_STOP, MCI_WAIT,
        reinterpret_cast<DWORD_PTR>(&genParms));
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