#include "ContextImpl.hpp"
#include "Logger.hpp"
#include "StringUtils.hpp"
#include "ErrorUtils.hpp"

#include "gl_core_3_3.h"
#include "wgl_ext.h"

#include <Shlwapi.h>

#include <stdexcept>

namespace glrage {

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ContextImpl* context = reinterpret_cast<ContextImpl*>(GetProp(hwnd, ContextImpl::PROP_CONTEXT));
    return context->windowProc(hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM _this) {
    ContextImpl* context = reinterpret_cast<ContextImpl*>(_this);
    return context->enumWindowsProc(hwnd);
}

ContextImpl::ContextImpl() {
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

void ContextImpl::init() {
    if (m_hglrc) {
        return;
    }

    // The exact point where the application will create its window is unknown,
    // but a valid OpenGL context is required at this point, so just create a
    // dummy window for now and transfer the context later.
    HWND m_hwndTmp = CreateWindow(L"STATIC", L"", WS_POPUP | WS_DISABLED, 0, 0, 1, 1,
        NULL, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(m_hwndTmp, SW_HIDE);

    m_hdc = GetDC(m_hwndTmp);
    if (!m_hdc) {
        ErrorUtils::error("Can't get device context", ErrorUtils::getWindowsErrorString());
    }

    int pf = ChoosePixelFormat(m_hdc, &m_pfd);
    if (!pf) {
        ErrorUtils::error("Can't choose pixel format", ErrorUtils::getWindowsErrorString());
    }

    if (!SetPixelFormat(m_hdc, pf, &m_pfd)) {
        ErrorUtils::error("Can't set pixel format", ErrorUtils::getWindowsErrorString());
    }

    m_hglrc = wglCreateContext(m_hdc);
    if (!m_hglrc || !wglMakeCurrent(m_hdc, m_hglrc)) {
        ErrorUtils::error("Can't create OpenGL context", ErrorUtils::getWindowsErrorString());
    }

    glClearColor(0, 0, 0, 0);
    glClearDepth(1);

    if (m_config.getBool("vsync", true)) {
        wglSwapIntervalEXT(1);
    }
}

void ContextImpl::attach(HWND hwnd) {
    if (m_hwnd) {
        return;
    }

    LOGF("Attaching to HWND %p", hwnd);

    m_hwnd = hwnd;

    ErrorUtils::setHWnd(m_hwnd);

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
        ErrorUtils::error("Can't set pixel format", ErrorUtils::getWindowsErrorString());
    }

    // set context on new window 
    if (!m_hglrc || !wglMakeCurrent(m_hdc, m_hglrc)) {
        ErrorUtils::error("Can't attach window to OpenGL context", ErrorUtils::getWindowsErrorString());
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

void ContextImpl::attach() {
    if (m_hwnd) {
        return;
    }

    m_pid = GetCurrentProcessId();
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
}

void ContextImpl::detach() {
    if (!m_hwnd) {
        return;
    }

    wglDeleteContext(m_hglrc);
    m_hglrc = nullptr;
    
    SetWindowLongPtr(m_hwnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(m_windowProc));
    m_windowProc = nullptr;

    m_hwnd = nullptr;
}

LRESULT ContextImpl::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        // Printscreen on Windows with OpenGL doesn't work in fullscreen, so hook the
        // key and implement screenshot saving to files.
        // For some reason, VK_SNAPSHOT doesn't generate WM_KEYDOWN events but only
        // WM_KEYUP. Works just as well, though.
        case WM_KEYUP:
            if (wParam == VK_SNAPSHOT) {
                m_screenshot.schedule(true);
                return TRUE;
            }
            break;

        // toggle fullscreen if alt + return is pressed
        case WM_SYSKEYDOWN:
            if (wParam == VK_RETURN && lParam & 1 << 29 && !(lParam & 1 << 30)) {
                toggleFullscreen();
                return TRUE;
            }
            break;

        // force default handling for some window messages when in windowed mode,
        // especially important for Tomb Raider
        case WM_MOVE:
        case WM_MOVING:
        case WM_SIZE:
        case WM_NCPAINT:
        case WM_SETCURSOR:
        case WM_GETMINMAXINFO:
        case WM_ERASEBKGND:
            if (!m_fullscreen) {
                return CallWindowProc(DefWindowProc, hwnd, msg, wParam, lParam);
            }
            break;
    }

    return CallWindowProc(m_windowProc, hwnd, msg, wParam, lParam);
}

BOOL ContextImpl::enumWindowsProc(HWND hwnd) {
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

bool ContextImpl::isFullscreen() {
    return m_fullscreen;
}

void ContextImpl::setFullscreen(bool fullscreen) {
    if (m_fullscreen == fullscreen) {
        return;
    }

    m_fullscreen = fullscreen;

    if (!m_hwnd) {
        return;
    }

    // change window style
    LONG style = GetWindowLong(m_hwnd, GWL_STYLE);
    LONG styleEx = GetWindowLong(m_hwnd, GWL_EXSTYLE);

    if (m_fullscreen) {
        style &= ~STYLE_WINDOW;
        styleEx &= ~STYLE_WINDOW_EX;
    } else {
        style |= STYLE_WINDOW;
        styleEx |= STYLE_WINDOW_EX;
    }

    SetWindowLong(m_hwnd, GWL_STYLE, style);
    SetWindowLong(m_hwnd, GWL_EXSTYLE, styleEx);

    // change window size
    int32_t width;
    int32_t height;

    if (m_fullscreen) {
        width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    } else {
        width = m_width;
        height = m_height;
    }

    ShowCursor(!m_fullscreen);

    setWindowSize(width, height);
}

void ContextImpl::toggleFullscreen() {
    setFullscreen(!m_fullscreen);
}

void ContextImpl::setDisplaySize(uint32_t width, uint32_t height) {
    LOGF("Display size: %dx%d", width, height);

    m_width = width;
    m_height = height;

    // update window size if not fullscreen
    if (!m_fullscreen) {
        setWindowSize(m_width, m_height);
    }
}

uint32_t ContextImpl::getDisplayWidth() {
    return m_width;
}

uint32_t ContextImpl::getDisplayHeight() {
    return m_height;
}

void ContextImpl::setWindowSize(uint32_t width, uint32_t height) {
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

uint32_t ContextImpl::getWindowWidth() {
    if (!m_hwnd) {
        return m_width;
    }

    GetClientRect(m_hwnd, &m_tmprect);
    return m_tmprect.right - m_tmprect.left;
}

uint32_t ContextImpl::getWindowHeight() {
    if (!m_hwnd) {
        return m_height;
    }

    GetClientRect(m_hwnd, &m_tmprect);
    return m_tmprect.bottom - m_tmprect.top;
}

void ContextImpl::setupViewport() {
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

void ContextImpl::swapBuffers() {
    glFinish();

    try {
        m_screenshot.captureScheduled();
    } catch (const std::exception& ex) {
        ErrorUtils::warning("Can't capture screenshot", ex);
        m_screenshot.schedule(false);
    }

    SwapBuffers(m_hdc);
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_render = false;
}

void ContextImpl::renderBegin() {
    m_render = true;
}

bool ContextImpl::isRendered() {
    return m_render;
}

HWND ContextImpl::getHWnd() {
    return m_hwnd;
}

std::wstring ContextImpl::getBasePath() {
    TCHAR path[MAX_PATH];
    HMODULE hModule = NULL;
    DWORD dwFlags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;

    if (!GetModuleHandleEx(dwFlags, reinterpret_cast<LPCWSTR>(&WindowProc), &hModule)) {
        throw std::runtime_error("Can't get module handle");
    }

    GetModuleFileName(hModule, path, sizeof(path));
    PathRemoveFileSpec(path);

    return path;
}

}