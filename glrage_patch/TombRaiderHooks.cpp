#include "TombRaiderHooks.hpp"

#include "Logger.hpp"

#include <cmath>

namespace glrage {

bool TombRaiderHooks::m_musicAlwaysLoop = false;
int32_t TombRaiderHooks::m_fpsTextX = 0;
int32_t TombRaiderHooks::m_fpsTextY = 0;

std::map<int32_t, int32_t> TombRaiderHooks::m_keyCodeMap = {
    {29, 157}, // CTRL
    {42, 54},  // SHIFT
    {56, 184}  // ALT
};

/** Tomb Raider sub pointers **/

// init sound system
TombRaiderSoundInit* TombRaiderHooks::m_tombSoundInit = nullptr;

// renders a line between two points
TombRaiderRenderLine* TombRaiderHooks::m_tombRenderLine = nullptr;

// creates overlay text
TombRaiderCreateOverlayText* TombRaiderHooks::m_tombCreateOverlayText = nullptr;

// changes the current horizontal field of view value
TombRaiderSetFOV* TombRaiderHooks::m_tombSetFOV = nullptr;

// renders the previously collected item
TombRaiderRenderCollectedItem* TombRaiderHooks::m_tombRenderCollectedItem =
    nullptr;

/** Tomb Raider var pointers **/

// Pointer to the key state table. If an entry is 1, then the key is pressed.
uint8_t** TombRaiderHooks::m_tombKeyStates = nullptr;

// Default key binding table. Maps the key scan codes to button IDs.
int16_t* TombRaiderHooks::m_tombDefaultKeyBindings = nullptr;

// Number of currently loaded audio samples.
int32_t* TombRaiderHooks::m_tombNumAudioSamples = nullptr;

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

// CD track to play after the current one has finished. Usually for ambiance
// tracks.
int32_t* TombRaiderHooks::m_tombCDTrackIDLoop = nullptr;

// CD loop flag. If TRUE, it re-plays m_tombCDTrackIDLoop after it finished.
BOOL* TombRaiderHooks::m_tombCDLoop = nullptr;

// Current music volume, ranging from 0 to 10.
uint32_t* TombRaiderHooks::m_tombCDVolume = nullptr;

// Number of CD track.
uint32_t* TombRaiderHooks::m_tombCDNumTracks = nullptr;

// MCI device ID.
MCIDEVICEID* TombRaiderHooks::m_tombMciDeviceID = nullptr;

// Auxiliary device ID.
uint32_t* TombRaiderHooks::m_tombAuxDeviceID = nullptr;

// Rendering width and height.
int32_t* TombRaiderHooks::m_tombRenderWidth = nullptr;
int32_t* TombRaiderHooks::m_tombRenderHeight = nullptr;

// Tick counter, based on the milliseconds since the system has been started.
// Each second has 60 ticks, so one tick lasts about 16.6 ms. The game logic
// runs on every other tick at most.
int32_t* TombRaiderHooks::m_tombTicks = nullptr;

// Window handle.
HWND* TombRaiderHooks::m_tombHwnd = nullptr;

// Keyboard hook handle.
HHOOK* TombRaiderHooks::m_tombHhk = nullptr;

int32_t TombRaiderHooks::soundInit()
{
    int32_t result = m_tombSoundInit();

    // Improves accuracy of the dB LUT, which is used to convert the volume to
    // hundredths of decibels (or, you know, thousandths of bels) for
    // DirectSound
    // The difference is probably barely audible, if at all, but it's still an
    // improvement.
    for (int i = 1; i < DECIBEL_LUT_SIZE; i++) {
        // original formula, 1 dB steps
        // m_tombDecibelLut[i] = 100 * static_cast<int32_t>(-90.0 -
        // std::log2(1.0 / i) * -10.0 / std::log2(0.5));
        // new formula, full dB precision
        m_tombDecibelLut[i] = static_cast<int32_t>(
            -9000.0 - std::log2(1.0 / i) * -1000.0 / std::log2(0.5));
    }

    return result;
}

int32_t TombRaiderHooks::convertVolumeToDecibel(int32_t volume)
{
    // convert volume to dB using the lookup table
    return m_tombDecibelLut[(volume & 0x7FFF) >> 6];
}

int32_t TombRaiderHooks::convertPanToDecibel(uint16_t pan)
{
    // Use the panning as the rotation ranging from 0 to 0xffff, convert it to
    // radians, apply the sine function and convert it to a value in the rage
    // -256 to 256 for the volume LUT.
    // Note that I used "DB_CONV_LUT_SIZE / 2" here, which limits the
    // attenuation for one channel to -50 dB to prevent it from being completely
    // silent.
    // This is a workaround for sounds that are played directly at Lara's
    // position, which often have incorrect pannings and flip more or less
    // randomly between the channels.
    int32_t result = static_cast<int32_t>(
        std::sin((pan / 32767.0) * M_PI) * (DECIBEL_LUT_SIZE / 2));

    if (result > 0) {
        result = -m_tombDecibelLut[DECIBEL_LUT_SIZE - result];
    } else if (result < 0) {
        result = m_tombDecibelLut[DECIBEL_LUT_SIZE + result];
    } else {
        result = 0;
    }

    return result;
}

LPDIRECTSOUNDBUFFER
TombRaiderHooks::soundPlaySample(
    int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan, bool loop)
{
    LOG_TRACE("%d %d %d %d %d", soundID, volume, pitch, pan, loop);

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

LPDIRECTSOUNDBUFFER
TombRaiderHooks::soundPlayOneShot(
    int32_t soundID, int32_t volume, int16_t pitch, uint16_t pan)
{
    LOG_TRACE("%d, %d, %d, %d", soundID, volume, pitch, pan);

    return soundPlaySample(soundID, volume, pitch, pan, false);
}

LPDIRECTSOUNDBUFFER
TombRaiderHooks::soundPlayLoop(int32_t soundID, int32_t volume, int16_t pitch,
    uint16_t pan, int32_t a5, int32_t a6, int32_t a7)
{
    LOG_TRACE(
        "%d, %d, %d, %d, %d, %d, %d", soundID, volume, pitch, pan, a5, a6, a7);

    return soundPlaySample(soundID, volume, pitch, pan, true);
}

void TombRaiderHooks::soundStopAll()
{
    LOG_TRACE("");

    // check if sound system is initialized
    if (!*m_tombSoundInit1 || !*m_tombSoundInit2) {
        return;
    }

    for (int32_t i = 0; i < *m_tombNumAudioSamples; i++) {
        TombRaiderAudioSample* sample = (*m_tombSampleTable)[i];

        if (sample) {
            sample->buffer->Stop();
        }
    }
}

LRESULT
TombRaiderHooks::keyboardProc(int32_t nCode, WPARAM wParam, LPARAM lParam)
{
    LOG_TRACE("%d, %d, %d", nCode, wParam, lParam);

    if (nCode < 0) {
        goto next;
    }

    uint32_t keyData = static_cast<uint32_t>(lParam);
    uint32_t scanCode = keyData >> 16 & 0xff;
    uint32_t extended = keyData >> 24 & 0x1;
    uint32_t pressed = ~keyData >> 31;

    // Remap DOS scan code for certain control keys.
    // This is also done in the original code, but at several different places
    // and also in reverse, which may be responsible for the stuck keys bug.
    // It's an ugly hack but still better than patching both the default key
    // mappings and the key names.
    if (m_keyCodeMap.find(scanCode) != m_keyCodeMap.end()) {
        scanCode = m_keyCodeMap[scanCode];
    } else if (extended) {
        scanCode += 128;
    }

    uint8_t* keyStates = *m_tombKeyStates;
    if (!keyStates) {
        goto next;
    }

    keyStates[scanCode] = pressed;
    keyStates[325] = pressed;

    // terminate ALT key hook so the menu won't pop up when jumping
    if (wParam == VK_MENU) {
        return 1;
    }

next:
    return CallNextHookEx(*m_tombHhk, nCode, wParam, lParam);
}

BOOL TombRaiderHooks::keyIsPressed(int32_t keyCode)
{
    int16_t keyBinding = m_tombDefaultKeyBindings[keyCode];
    uint8_t* keyStates = *m_tombKeyStates;
    return keyStates[keyBinding];
}

BOOL TombRaiderHooks::renderHealthBar(int32_t health)
{
    renderBar(health, false);
    return TRUE;
}

BOOL TombRaiderHooks::renderAirBar(int32_t air)
{
    renderBar(air, true);
    return TRUE;
}

BOOL TombRaiderHooks::renderCollectedItem(
    int32_t x, int32_t y, int32_t scale, int16_t itemID, int16_t brightness)
{
    return m_tombRenderCollectedItem(
        x, y, getOverlayScale(scale), itemID, brightness);
}

void* TombRaiderHooks::createFPSText(
    int16_t x, int16_t y, int16_t a3, const char* text)
{
    if (getOverlayScale() > 1) {
        x = m_fpsTextX;
        y = m_fpsTextY;
    }

    return m_tombCreateOverlayText(x, y, a3, text);
}

int16_t TombRaiderHooks::setFOV(int16_t fov)
{
    double aspectRatio =
        *m_tombRenderWidth / static_cast<double>(*m_tombRenderHeight);

    // convert to radians ("fov" is in degrees mapped from 0 to 32760)
    double hFovRad = fov * M_PI / 32760;

    // convert horizontal FOV to vertical
    double vFovRad = 2 * std::atan(aspectRatio * std::tan(hFovRad / 2));

    // convert back to degrees
    fov = static_cast<int16_t>(std::round((vFovRad / M_PI) * 32760));

    // call original setFOV function that expects a horizontal FOV
    return m_tombSetFOV(fov);
}

BOOL TombRaiderHooks::musicPlayRemap(int16_t trackID)
{
    LOG_TRACE("%d", trackID);

    // stop CD play on track ID 0
    if (trackID == 0) {
        musicStop();
        return FALSE;
    }

    // ignore redundant PSX ambience track (replaced by 57)
    if (trackID == 5) {
        return FALSE;
    }

    // set current track ID
    *m_tombCDTrackID = trackID;

    return musicPlay(trackID);
}

BOOL TombRaiderHooks::musicPlayLoop()
{
    LOG_TRACE("");

    // cancel if there's currently no looping track set
    if (*m_tombCDLoop && *m_tombCDTrackIDLoop > 0) {
        musicPlay(*m_tombCDTrackIDLoop);
        return FALSE;
    }

    return *m_tombCDLoop;
}

BOOL TombRaiderHooks::musicPlay(int16_t trackID)
{
    LOG_TRACE("%d", trackID);

    // don't play music tracks if volume is set to 0
    if (!*m_tombCDVolume) {
        return FALSE;
    }

    // don't try to play data track
    if (trackID < 2) {
        return FALSE;
    }

    // set looping track ID for ambience tracks
    if (m_musicAlwaysLoop || trackID >= 57) {
        *m_tombCDTrackIDLoop = trackID;
    }

    // this one will always be set back to true on the next tick
    *m_tombCDLoop = FALSE;

    // Calculate volume from current volume setting. The original code used
    // the hardcoded value 0x400400, which equals a volume of 25%.
    uint32_t volume = *m_tombCDVolume * 0xffff / 10;
    volume |= volume << 16;
    auxSetVolume(*m_tombAuxDeviceID, volume);

    // configure time format
    MCI_SET_PARMS setParms;
    setParms.dwTimeFormat = MCI_FORMAT_TMSF;
    if (mciSendCommandA(*m_tombMciDeviceID, MCI_SET, MCI_SET_TIME_FORMAT,
            reinterpret_cast<DWORD_PTR>(&setParms))) {
        return FALSE;
    }

    // send play command
    DWORD_PTR dwFlags = MCI_NOTIFY | MCI_FROM;
    MCI_PLAY_PARMS openParms;
    openParms.dwCallback = reinterpret_cast<DWORD_PTR>(*m_tombHwnd);
    openParms.dwFrom = trackID;

    // MCI can't play beyond the last track, so omit MCI_TO in that case
    if (trackID != *m_tombCDNumTracks) {
        openParms.dwTo = trackID + 1;
        dwFlags |= MCI_TO;
    }

    if (mciSendCommandA(*m_tombMciDeviceID, MCI_PLAY, dwFlags,
            reinterpret_cast<DWORD_PTR>(&openParms))) {
        return FALSE;
    }

    return TRUE;
}

BOOL TombRaiderHooks::musicStop()
{
    LOG_TRACE("");

    *m_tombCDTrackID = 0;
    *m_tombCDTrackIDLoop = 0;
    *m_tombCDLoop = FALSE;

    // The original code used MCI_PAUSE, probably to reduce latency when
    // switching
    // tracks. But we'll use MCI_STOP here, since it's expected to use an MCI
    // wrapper with nearly zero latency anyway.
    MCI_GENERIC_PARMS genParms;
    return !mciSendCommandA(*m_tombMciDeviceID, MCI_STOP, MCI_WAIT,
        reinterpret_cast<DWORD_PTR>(&genParms));
}

BOOL TombRaiderHooks::musicSetVolume(int16_t volume)
{
    LOG_TRACE("%d", volume);

    uint32_t volumeAux = volume * 0xffff / 0xff;
    volumeAux |= volumeAux << 16;
    auxSetVolume(*m_tombAuxDeviceID, volumeAux);

    return TRUE;
}

void TombRaiderHooks::soundSetVolume(LPDIRECTSOUNDBUFFER buffer, int32_t volume)
{
    if (buffer) {
        buffer->SetVolume(convertVolumeToDecibel(volume));
    }
}

void TombRaiderHooks::soundSetPan(LPDIRECTSOUNDBUFFER buffer, int32_t pan)
{
    if (buffer) {
        buffer->SetPan(convertPanToDecibel(pan));
    }
}

void TombRaiderHooks::renderBar(int32_t value, bool air)
{
    const int32_t p1 = -100;
    const int32_t p2 = -200;
    const int32_t p3 = -300;
    const int32_t p4 = -400;

    const int32_t valueMax = 100;

    const int32_t colorBarSize = 5;
    const int32_t colorBar[2][colorBarSize] = {
        {8, 11, 8, 6, 24}, {32, 41, 32, 19, 21}};

    const int32_t colorBorder1 = 19;
    const int32_t colorBorder2 = 17;
    const int32_t colorBackground = 0;

    int32_t scale = getOverlayScale();
    int32_t width = valueMax * scale;
    int32_t height = 5 * scale;

    int32_t x = 8 * scale;
    int32_t y = 8 * scale;

    // place air bar on the right
    if (air) {
        x = *m_tombRenderWidth - width - x;
    }

    int32_t padding = 2;
    int32_t top = y - padding;
    int32_t left = x - padding;
    int32_t bottom = top + height + padding + 1;
    int32_t right = left + width + padding + 1;

    // set offset for FPS text
    if (!air) {
        m_fpsTextX = left;
        m_fpsTextY = bottom + 24;
    }

    // background
    for (int32_t i = 1; i < height + 3; i++) {
        m_tombRenderLine(
            left + 1, top + i, right, top + i, p1, colorBackground);
    }

    // top / left border
    m_tombRenderLine(left, top, right + 1, top, p2, colorBorder1);
    m_tombRenderLine(left, top, left, bottom, p2, colorBorder1);

    // bottom / right border
    m_tombRenderLine(left + 1, bottom, right, bottom, p2, colorBorder2);
    m_tombRenderLine(right, top, right, bottom, p2, colorBorder2);

    const int32_t blinkInterval = 20;
    const int32_t blinkThresh = 20;
    int32_t blinkTime = *m_tombTicks % blinkInterval;
    bool blink = value <= blinkThresh && blinkTime > blinkInterval / 2;

    if (value && !blink) {
        width -= (valueMax - value) * scale;

        top = y;
        left = x;
        bottom = top + height;
        right = left + width;

        for (int32_t i = 0; i < height; i++) {
            int32_t colorType = air ? 1 : 0;
            int32_t colorIndex = i * colorBarSize / height;
            m_tombRenderLine(left, top + i, right, top + i, p4,
                colorBar[colorType][colorIndex]);
        }
    }
}

int32_t TombRaiderHooks::getOverlayScale()
{
    return getOverlayScale(1);
}

int32_t TombRaiderHooks::getOverlayScale(int32_t base)
{
    double result = static_cast<double>(*m_tombRenderWidth);
    result *= base;
    result /= 800.0;

    // only scale up, not down
    if (result < base) {
        result = base;
    }

    return static_cast<int32_t>(std::round(result));
}

} // namespace glrage