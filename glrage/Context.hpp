#pragma once

#include "Screenshot.hpp"
#include "Config.hpp"

#include <Windows.h>
#include <cstdint>
#include <string>

namespace glrage {

class Context {
public:
    // singleton accessor
    static Context& getInstance() {
        static Context instance;
        return instance;
    }

    void init();
    void attach(HWND hwnd);
    void attach();
    void detach();
    LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    BOOL enumWindowsProc(HWND hwnd);
    bool isFullscreen();
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    void setDisplaySize(uint32_t width, uint32_t height);
    uint32_t getDisplayWidth();
    uint32_t getDisplayHeight();
    void setWindowSize(uint32_t width, uint32_t height);
    uint32_t getWindowWidth();
    uint32_t getWindowHeight();
    void setupViewport();
    void swapBuffers();
    void renderBegin();
    bool isRendered();
    HWND getHWnd();

private:
    // private singleton constructor and operators
    Context();
    Context(Context const&) = delete;
    void operator=(Context const&) = delete;

    // private methods
    void fixWindowRatio(WPARAM edge, LPRECT prect);
    void error(const std::string& message);

    // config object
    Config m_config;

    // window handle
    HWND m_hwnd;
    HWND m_hwndTmp;

    // GDI device context
    HDC m_hdc;

    // OpenGL context handle
    HGLRC m_hglrc;

    // Attached process ID
    DWORD m_pid;

    // Pixel format
    PIXELFORMATDESCRIPTOR m_pfd;

    // Original window properties
    WNDPROC m_windowProc;

    // fullscreen flag
    bool m_fullscreen;

    // rendering flag
    bool m_render;

    // screenshot object
    Screenshot m_screenshot;

    // temporary rectangle
    RECT m_tmprect;

    // DirectDraw display mode dimension
    uint32_t m_width;
    uint32_t m_height;
};

}