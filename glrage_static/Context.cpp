#include "Context.hpp"
#include "Logger.hpp"
#include "StringUtils.hpp"

#include "gl_core_3_3.h"
#include "wgl_ext.h"

#include <mciapi.h>
#include <stdexcept>

#define PROP_CONTEXT "Context.this"

namespace glrage {

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Context* context = reinterpret_cast<Context*>(GetProp(hwnd, PROP_CONTEXT));
    return context->windowProc(hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM _this) {
    Context* context = reinterpret_cast<Context*>(_this);
    return context->enumWindowsProc(hwnd);
}

Context::Context() :
    m_config("Context"),
    m_hwnd(nullptr),
    m_hwndTmp(nullptr),
    m_hdc(nullptr),
    m_hglrc(nullptr),
    m_pid(0),
    m_windowProc(nullptr),
    m_fullscreen(false),
    m_render(false),
    m_width(0),
    m_height(0)
{
    SetRectEmpty(&m_tmprect);

    // set pixel format
    m_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    m_pfd.nVersion = 1;
    m_pfd.iPixelType = PFD_TYPE_RGBA;
    m_pfd.cColorBits = 32;
    m_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    m_pfd.cDepthBits = 32;
    m_pfd.iLayerType = PFD_MAIN_PLANE;
}

void Context::init() {
    if (m_hglrc) {
        return;
    }

    // The exact point where the application will create its window is unknown,
    // but a valid OpenGL context is required at this point, so just create a
    // dummy window for now and transfer the context later.
    HWND m_hwndTmp = CreateWindow("STATIC", "", WS_POPUP | WS_DISABLED, 0, 0, 1, 1,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(m_hwndTmp, SW_HIDE);

    m_hdc = GetDC(m_hwndTmp);
    if (!m_hdc) {
        error("Can't get device context.");
    }

    int pf = ChoosePixelFormat(m_hdc, &m_pfd);
    if (!pf) {
        error("Can't choose pixel format.");
    }

    if (!SetPixelFormat(m_hdc, pf, &m_pfd)) {
        error("Can't set pixel format.");
    }

    m_hglrc = wglCreateContext(m_hdc);
    if (!m_hglrc || !wglMakeCurrent(m_hdc, m_hglrc)) {
        error("Can't create OpenGL context.");
    }

    glClearColor(0, 0, 0, 0);
    glClearDepth(1);

    if (m_config.getBool("vsync", true)) {
        wglSwapIntervalEXT(1);
    }
}

void Context::attach(HWND hwnd) {
    if (m_hwnd) {
        return;
    }

    LOGF("Attaching to HWND %p", hwnd);

    m_hwnd = hwnd;

    // get window procedure pointer and replace it with custom procedure
    SetProp(m_hwnd, PROP_CONTEXT, this);
    m_windowProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(m_hwnd, GWL_WNDPROC));
    SetWindowLongPtr(m_hwnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));

    // detach from current window
    wglMakeCurrent(NULL, NULL);

    // destroy temporary window
    if (m_hwndTmp) {
        DestroyWindow(m_hwndTmp);
        m_hwndTmp = nullptr;
    }
    
    // get DC of window
    m_hdc = GetDC(m_hwnd);
    INT pf = ChoosePixelFormat(m_hdc, &m_pfd);
    if (!pf || !SetPixelFormat(m_hdc, pf, &m_pfd)) {
        error("Can't set pixel format.");
    }

    // set context on new window 
    if (!m_hglrc || !wglMakeCurrent(m_hdc, m_hglrc)) {
        error("Can't attach window to OpenGL context.");
    }

    // apply previously applied window size
    if (m_width > 0 && m_height > 0) {
        setWindowSize(m_width, m_height);
    }

    // apply previously applied fullscreen mode
    if (m_fullscreen) {
        setFullscreen(m_fullscreen);
    }
}

void Context::attach() {
    if (m_hwnd) {
        return;
    }

    m_pid = GetCurrentProcessId();
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
}

void Context::detach() {
    if (!m_hwnd) {
        return;
    }

    wglDeleteContext(m_hglrc);
    m_hglrc = nullptr;
    
    SetWindowLongPtr(m_hwnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(m_windowProc));
    m_windowProc = nullptr;

    m_hwnd = nullptr;
}

LRESULT Context::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Printscreen on Windows with OpenGL doesn't work in fullscreen, so hook the
    // key and implement screenshot saving to files.
    // For some reason, VK_SNAPSHOT doesn't generate WM_KEYDOWN events but only
    // WM_KEYUP. Works just as well, though.
    if (msg == WM_KEYUP && wParam == VK_SNAPSHOT) {
        m_screenshot.schedule();
        return TRUE;
    }

    // toggle fullscreen if alt + return is pressed
    if (msg == WM_SYSKEYDOWN && wParam == VK_RETURN && lParam & 1 << 29 && !(lParam & 1 << 30)) {
        toggleFullscreen();
        return TRUE;
    }

    return m_windowProc(hwnd, msg, wParam, lParam);
}

BOOL Context::enumWindowsProc(HWND hwnd) {
    // ignore invisible windows
    if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_VISIBLE)) {
        return TRUE;
    }

    // check if the window is using the correct thread ID
    DWORD pidwin;
    GetWindowThreadProcessId(hwnd, &pidwin);
    if (pidwin != m_pid) {
        return TRUE;
    }

    // attach to window
    attach(hwnd);

    return FALSE;
}

bool Context::isFullscreen() {
    return m_fullscreen;
}

void Context::setFullscreen(bool fullscreen) {
    m_fullscreen = fullscreen;

    if (!m_hwnd) {
        return;
    }

    int32_t width;
    int32_t height;

    if (m_fullscreen) {
        width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    } else {
        width = m_width;
        height = m_height;
    }

    setWindowSize(width, height);
}

void Context::toggleFullscreen() {
    setFullscreen(!m_fullscreen);
}

void Context::setDisplaySize(uint32_t width, uint32_t height) {
    LOGF("Display size: %dx%d", width, height);

    m_width = width;
    m_height = height;

    // update window size if not fullscreen
    if (!m_fullscreen) {
        setWindowSize(m_width, m_height);
    }
}

uint32_t Context::getDisplayWidth() {
    return m_width;
}

uint32_t Context::getDisplayHeight() {
    return m_height;
}

void Context::setWindowSize(uint32_t width, uint32_t height) {
    if (!m_hwnd) {
        return;
    }

    LOGF("Window size: %dx%d", width, height);

    int32_t desktopWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int32_t desktopHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    // if windowed mode is active and the display mode is as large as the deskop,
    // then divide the size into half so it won't go into fullscreen
    if (!m_fullscreen && width == desktopWidth && height == desktopHeight) {
        width /= 2;
        height /= 2;
    }

    // center window on desktop
    int32_t left = desktopWidth / 2 - width / 2;
    int32_t top = desktopHeight / 2 - height / 2;

    SetWindowPos(m_hwnd, HWND_NOTOPMOST, left, top, width, height,
        SWP_SHOWWINDOW | SWP_FRAMECHANGED);
}

uint32_t Context::getWindowWidth() {
    if (!m_hwnd) {
        return m_width;
    }

    GetClientRect(m_hwnd, &m_tmprect);
    return m_tmprect.right - m_tmprect.left;
}

uint32_t Context::getWindowHeight() {
    if (!m_hwnd) {
        return m_height;
    }

    GetClientRect(m_hwnd, &m_tmprect);
    return m_tmprect.bottom - m_tmprect.top;
}

void Context::setupViewport() {
    UINT vpWidth = getWindowWidth();
    UINT vpHeight = getWindowHeight();

    // default to bottom left corner of the window
    UINT vpX = 0;
    UINT vpY = 0;

    UINT hw = m_height * vpWidth;
    UINT wh = m_width * vpHeight;

    // create viewport offset if the window has a different
    // aspect ratio than the current display mode
    if (hw > wh) {
        UINT wMax = wh / m_height;
        vpX = (vpWidth - wMax) / 2;
        vpWidth = wMax;
    } else if (hw < wh) {
        UINT hMax = hw / m_width;
        vpY = (vpHeight - hMax) / 2;
        vpHeight = hMax;
    }

    glViewport(vpX, vpY, vpWidth, vpHeight);
}

void Context::swapBuffers() {
    glFinish();

    try {
        m_screenshot.captureScheduled();
    } catch (const std::exception& ex) {
        error(ex.what());
    }

    SwapBuffers(m_hdc);
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Context::renderBegin() {
    m_render = true;
}

bool Context::isRendered() {
    bool result = m_render;
    m_render = false;
    return m_render;
}

// Create a string with last error message
std::string GetLastErrorStdStr() {
    DWORD error = GetLastError();
    if (error) {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL
        );

        if (bufLen) {
            LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
            std::string result(lpMsgStr, lpMsgStr + bufLen);
            LocalFree(lpMsgBuf);
            return result;
        }
    }
    return std::string();
}

void Context::error(const std::string& message) {
    std::string errorString = GetLastErrorStdStr();

    if (!errorString.empty()) {
        MessageBox(m_hwnd, (message + " " + errorString).c_str(), "Error", MB_OK);
    } else {
        MessageBox(m_hwnd, message.c_str(), "Error", MB_OK);
    }

    ExitProcess(1);
}

HWND Context::getHWnd() {
    return m_hwnd;
}

}