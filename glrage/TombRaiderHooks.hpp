#pragma once

#include <Windows.h>
#include <mmeapi.h>
#include <dsound.h>
#include <cstdint>

namespace glrage {

#pragma pack(push)
#pragma pack(1)
struct TombRaiderAudioSample {
    uint32_t data;
    uint16_t length;
    uint16_t bitsPerSample;
    uint16_t channels;
    uint16_t unknown1;
    uint16_t sampleRate;
    uint16_t unknown2;
    uint16_t channels2;
    uint32_t unknown3;
    uint16_t volume;
    uint32_t pan;
    uint16_t unknown4;
    LPDIRECTSOUNDBUFFER buffer;
    uint16_t unknown5;
};
#pragma pack(pop)

// Tomb Raider sub types
typedef int32_t TombRaiderSoundInit();
typedef int32_t TombRaiderCDStop();
typedef int32_t TombRaiderCDPlay(uint32_t);
typedef void TombRaiderKeyEvent(int32_t, int32_t, int32_t);

class TombRaiderHooks {
public:
    static int32_t soundInit();
    static void setVolume(LPDIRECTSOUNDBUFFER buffer, int32_t volume);
    static void setPan(LPDIRECTSOUNDBUFFER buffer, int32_t pan);
    static LPDIRECTSOUNDBUFFER playOneShot(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan);
    static LPDIRECTSOUNDBUFFER playLoop(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, int32_t a5, int32_t a6, int32_t a7);
    static int32_t playCDTrack(int16_t trackID);
    static void keyEvent(int32_t extended, int32_t code, int32_t pressed);

    static const int32_t DECIBEL_LUT_SIZE = 512;

    // Tomb Raider subs
    static TombRaiderSoundInit* m_tombSoundInit;
    static TombRaiderCDStop* m_tombCDStop;
    static TombRaiderCDPlay* m_tombCDPlay;
    static TombRaiderKeyEvent* m_tombKeyEvent;

    // Tomb Raider vars
    static TombRaiderAudioSample*** m_tombSampleTable;
    static int32_t* m_tombSoundInit1;
    static int32_t* m_tombSoundInit2;
    static int32_t* m_tombDecibelLut;
    static int32_t* m_tombTrackID;
    static int32_t* m_tombTrackIDLoop;
    static uint8_t** m_tombKeyStates;

private:
    static LPDIRECTSOUNDBUFFER playSample(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, bool loop);
    static int32_t convertPanToDecibel(uint16_t pan);
    static int32_t convertVolumeToDecibel(int32_t volume);
};

}