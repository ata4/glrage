#include "DirectDraw.hpp"

#include "GLRage.hpp"
#include "Logger.hpp"

#include <string>

using ddraw::DirectDraw;

static Context& context = GLRageGetContext();

HRESULT HandleException() {
    try {
        throw;
    } catch (const std::runtime_error& ex) {
        MessageBox(context.getHWnd(), ex.what(), nullptr, MB_OK | MB_ICONERROR);
        return DDERR_GENERIC;
    } catch (const std::logic_error& ex) {
        MessageBox(context.getHWnd(), ex.what(), nullptr, MB_OK | MB_ICONERROR);
        return DDERR_GENERIC;
    }
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) {
    TRACEF("DirectDraw DllMain(%p,%d)",hModule, dwReason); 
    return TRUE;
}

HRESULT WINAPI DirectDrawCreate(GUID FAR *lpGUID, LPDIRECTDRAW FAR *lplpDD, IUnknown FAR *pUnkOuter) {
    TRACEF("DirectDrawCreate");

    context.init();
    context.attach();

    try {
        *lplpDD = new DirectDraw();
    } catch (...) {
        return HandleException();
    }

    return DD_OK;
}