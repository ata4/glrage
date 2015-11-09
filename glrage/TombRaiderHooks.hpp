#pragma once

#include <Windows.h>
#include <mmeapi.h>
#include <dsound.h>
#include <cstdint>
#include <map>

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
typedef BOOL TombRaiderRenderLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t a5, int8_t color);
typedef BOOL TombRaiderRenderCollectedItem(int32_t x, int32_t y, int32_t scale, int16_t itemID, int16_t brightness);
typedef void* TombRaiderCreateOverlayText(int16_t x, int16_t y, int16_t a3, const char* text);

class TombRaiderHooks {
public:
    static int32_t soundInit();
    static void soundSetVolume(LPDIRECTSOUNDBUFFER buffer, int32_t volume);
    static void soundSetPan(LPDIRECTSOUNDBUFFER buffer, int32_t pan);
    static LPDIRECTSOUNDBUFFER soundPlayOneShot(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan);
    static LPDIRECTSOUNDBUFFER soundPlayLoop(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, int32_t a5, int32_t a6, int32_t a7);
    static void soundStopAll();
    static BOOL musicPlayRemap(int16_t trackID);
    static BOOL musicPlayLoop();
    static BOOL musicPlay(int16_t trackID);
    static BOOL musicStop();
    static BOOL musicSetVolume(int16_t volume);
    static LRESULT keyboardProc(int32_t nCode, WPARAM wParam, LPARAM lParam);
    static BOOL keyIsPressed(int32_t keyCode);
    static BOOL renderHealthBar(int32_t health);
    static BOOL renderAirBar(int32_t air);
    static BOOL renderCollectedItem(int32_t x, int32_t y, int32_t scale, int16_t itemID, int16_t brightness);
    static void* createFPSText(int16_t x, int16_t y, int16_t a3, const char* text);

    static const int32_t DECIBEL_LUT_SIZE = 512;

    // Tomb Raider subs
    static TombRaiderSoundInit* m_tombSoundInit;
    static TombRaiderRenderLine* m_tombRenderLine;
    static TombRaiderRenderCollectedItem* m_tombRenderCollectedItem;
    static TombRaiderCreateOverlayText* m_tombCreateOverlayText;

    // Tomb Raider vars
    static uint8_t** m_tombKeyStates;
    static int16_t* m_tombDefaultKeyBindings;
    static int32_t* m_tombNumAudioSamples;
    static TombRaiderAudioSample*** m_tombSampleTable;
    static BOOL* m_tombSoundInit1;
    static BOOL* m_tombSoundInit2;
    static int32_t* m_tombDecibelLut;
    static int32_t* m_tombCDTrackID;
    static int32_t* m_tombCDTrackIDLoop;
    static BOOL* m_tombCDLoop;
    static uint32_t* m_tombCDVolume;
    static uint32_t* m_tombCDNumTracks;
    static MCIDEVICEID* m_tombMciDeviceID;
    static uint32_t* m_tombAuxDeviceID;
    static int32_t* m_tombRenderWidth;
    static int32_t* m_tombTicks;
    static HWND* m_tombHwnd;
    static HHOOK* m_tombHhk;

    // other vars
    static bool m_ub;

private:
    static LPDIRECTSOUNDBUFFER soundPlaySample(int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, bool loop);
    static int32_t convertPanToDecibel(uint16_t pan);
    static int32_t convertVolumeToDecibel(int32_t volume);
    static void renderBar(int32_t value, bool air);
    static int32_t getOverlayScale();
    static int32_t getOverlayScale(int32_t base);

    static int32_t m_fpsTextX;
    static int32_t m_fpsTextY;
    static std::map<int32_t, int32_t> m_keyCodeMap;
};

}