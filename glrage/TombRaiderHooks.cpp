#include "TombRaiderHooks.hpp"

#include "Logger.hpp"

#include <cmath>

namespace glrage {

/** Tomb Raider sub pointers **/

// init sound system
TombRaiderSoundInit* TombRaiderHooks::m_tombSoundInit = nullptr;

// stop current CD track
TombRaiderCDStop* TombRaiderHooks::m_tombCDStop = nullptr;

// play CD track
TombRaiderCDPlay* TombRaiderHooks::m_tombCDPlay = nullptr;

/** Tomb Raider var pointers **/

// CD track currently played
int32_t* TombRaiderHooks::m_tombTrackID = nullptr;

// CD track to play after the current one has finished. Usually for ambiance tracks.
int32_t* TombRaiderHooks::m_tombTrackIDLoop = nullptr;

// Key state table, one byte per key.
uint8_t** TombRaiderHooks::m_tombKeyStates = nullptr;

// audio sample pointer table
TombRaiderAudioSample*** TombRaiderHooks::m_tombSampleTable = nullptr;

// Sound init booleans. There are two for some reason and both are set to 1 at
// the same location.
int32_t* TombRaiderHooks::m_tombSoundInit1 = nullptr;
int32_t* TombRaiderHooks::m_tombSoundInit2 = nullptr;

// pointer to linear to log lookup table for decibel conversion
int32_t* TombRaiderHooks::m_tombDecibelLut = nullptr;

int32_t TombRaiderHooks::soundInit(){
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

int32_t TombRaiderHooks::playCDTrack(int16_t trackID) {
    //////////////////////////////////////////////////////////////
    ///////////////////// LEVEL TRACK ID LIST ////////////////////
    //////////////////////////////////////////////////////////////
    //
    // Music tracks (PSX)
    // 2    Main Title (also present on PC)                 [3:19]
    // 3    Chorus 1                                        [1:06]
    // 4    Main Title (short version)                      [2:06]
    // 5    Cave Ambience (played in most levels on PSX)    [2:02]
    // 6    Chase Theme 1                                   [1:19]
    // 7    Discovery Theme 1                               [0:58]
    // 8    Battle Theme 1                                  [1:10]
    // 9    Discovery Theme 2                               [0:42]
    // 10   Mystery Theme 1                                 [1:21]
    // 11   Danger 1                                        [0:10]
    // 12   Danger 2 (slowed)                               [0:17]
    // 13   Secret Chime 1 (audio sample on PC)             [0:07]
    // 14   Secret Chime 2 (slowed, unused)                 [0:14]
    // 15   Chorus 2                                        [0:13]
    // 16   Battle Theme 2                                  [0:41]
    // 17   Chorus 3                                        [0:45]
    // 18   Chorus 4                                        [0:27]
    // 19   Chorus 5 (unused)                               [1:31]
    // 20   Chase Theme 2                                   [0:56]
    // 21   Mystery Theme 2                                 [0:43]
    //
    // Cutscene tracks
    // 22 (PC: 7)  Lara and Natla in the Machine Room       [1:05]
    // 23 (PC: 8)  Lara and Larson                          [1:00]
    // 24 (PC: 9)  Natla activates the Pyramid              [0:17]
    // 25 (PC: 10) Lara in the Tomb of Tihocan              [0:39]
    //
    // Ingame speech tracks (audio samples on PC)
    // 26-50  Lara tutorial speech (Lara's Home)
    // 51-56  NPC speech
    //
    // Ambience tracks (PC, names from TRLE)
    // 57 (PC: 3)  DERELICT (same as 5 on PSX)              [2:02]
    // 58 (PC: 4)  WATER                                    [3:05]
    // 59 (PC: 5)  WIND                                     [3:11]
    // 60 (PC: 6)  HEARTBT                                  [3:10]

    LOGF("Playing CD track: %d (current: %d, loop: %d)", trackID, *m_tombTrackID, *m_tombTrackIDLoop);

    // stop CD play on track ID 0
    if (trackID == 0) {
        m_tombCDStop();
        *m_tombTrackIDLoop = 0;
        return 0;
    }

    // ignore redundant PSX ambience track
    if (trackID == 5) {
        return 0;
    }

    // save loop track ID (overridden by tombCDPlay)
    int32_t trackIDLoop = *m_tombTrackIDLoop;

    // set and play new track ID
    int32_t result = m_tombCDPlay(trackID);
    *m_tombTrackID = trackID;

    // restore loop track ID if the current track is not an ambience track
    if (trackID < 57) {
        *m_tombTrackIDLoop = trackIDLoop;
    }

    return result;
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