#pragma once

#include <Windows.h>
#include <cstdint>

#ifdef GLR_EXPORTS
#define GLRAPI __declspec(dllexport)
#else
#define GLRAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

void GLRAPI GLRageInit();
void GLRAPI GLRageAttach();
void GLRAPI GLRageDetach();
bool GLRAPI GLRageIsFullscreen();
void GLRAPI GLRageSetFullscreen(bool fullscreen);
void GLRAPI GLRageToggleFullscreen();
void GLRAPI GLRageSetDisplaySize(uint32_t width, uint32_t height);
uint32_t GLRAPI GLRageGetDisplayWidth();
uint32_t GLRAPI GLRageGetDisplayHeight();
void GLRAPI GLRageSetWindowSize(uint32_t width, uint32_t height);
uint32_t GLRAPI GLRageGetWindowWidth();
uint32_t GLRAPI GLRageGetWindowHeight();
void GLRAPI GLRageSetupViewport();
void GLRAPI GLRageSwapBuffers();
void GLRAPI GLRageRenderBegin();
bool GLRAPI GLRageIsRendered();
HWND GLRAPI GLRageGetHWnd();

#ifdef __cplusplus
}
#endif