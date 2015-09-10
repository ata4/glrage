#include "glrage.h"
#include "MemoryPatcher.hpp"
#include "Context.hpp"
#include "Logger.hpp"

using glrage::Context;
using glrage::MemoryPatcher;

Context& ctx = Context::getInstance();

const char* DLLMAIN_REASON[] {
    "DLL_PROCESS_DETACH",
    "DLL_PROCESS_ATTACH",
    "DLL_THREAD_ATTACH",
    "DLL_THREAD_DETACH"
};

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved) {
    TRACEF("GLRage DllMain(%p,%s,%08X)", hInst, DLLMAIN_REASON[dwReason], lpReserved); 

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            MemoryPatcher::patch();
            break;
    }

    return TRUE;
}

extern "C" {

void GLRAPI GLRageInit() {
    ctx.init();
}

void GLRAPI GLRageAttach() {
    ctx.attach();
}

void GLRAPI GLRageDetach() {
    ctx.detach();
}

bool GLRAPI GLRageIsFullscreen() {
    return ctx.isFullscreen();
}

void GLRAPI GLRageSetFullscreen(bool fullscreen) {
    ctx.setFullscreen(fullscreen);
}

void GLRAPI GLRageToggleFullscreen() {
    ctx.toggleFullscreen();
}

void GLRAPI GLRageSetDisplaySize(UINT width, UINT height) {
    ctx.setDisplaySize(width, height);
}

uint32_t GLRAPI GLRageGetDisplayWidth() {
    return ctx.getDisplayWidth();
}

uint32_t GLRAPI GLRageGetDisplayHeight() {
    return ctx.getDisplayHeight();
}

void GLRAPI GLRageSetWindowSize(UINT width, UINT height) {
    ctx.setWindowSize(width, height);
}

uint32_t GLRAPI GLRageGetWindowWidth() {
    return ctx.getWindowWidth();
}

uint32_t GLRAPI GLRageGetWindowHeight() {
    return ctx.getWindowHeight();
}

void GLRAPI GLRageSetupViewport() {
    ctx.setupViewport();
}

void GLRAPI GLRageSwapBuffers() {
    ctx.swapBuffers();
}

void GLRAPI GLRageRenderBegin() {
    ctx.renderBegin();
}

bool GLRAPI GLRageIsRendered() {
    return ctx.isRendered();
}

HWND GLRAPI GLRageGetHWnd() {
    return ctx.getHWnd();
}

}