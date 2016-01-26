#pragma once

#include "Context.hpp"
#include "Screenshot.hpp"
#include "Config.hpp"

namespace glrage {

class ContextImpl : public Context {
public:
    ContextImpl();
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
    std::string getBasePath();

private:
    // constants
    static const LONG STYLE_WINDOW = WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPED | WS_SYSMENU;
    static const LONG STYLE_WINDOW_EX = WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE;

    // config object
    Config m_config{ "Context" };

    // window handle
    HWND m_hwnd = nullptr;
    HWND m_hwndTmp = nullptr;

    // GDI device context
    HDC m_hdc = nullptr;

    // OpenGL context handle
    HGLRC m_hglrc = nullptr;

    // Attached process ID
    DWORD m_pid = 0;

    // Pixel format
    PIXELFORMATDESCRIPTOR m_pfd;

    // Original window properties
    WNDPROC m_windowProc = nullptr;

    // fullscreen flag
    bool m_fullscreen = false;

    // rendering flag
    bool m_render = false;

    // screenshot object
    Screenshot m_screenshot;

    // temporary rectangle
    RECT m_tmprect{ 0 };

    // DirectDraw display mode dimension
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

}