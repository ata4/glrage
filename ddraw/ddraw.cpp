#include "DirectDraw.hpp"

#include "ErrorUtils.hpp"
#include "GLRage.hpp"
#include "Logger.hpp"

#include <string>

using ddraw::DirectDraw;
using glrage::ErrorUtils;

static Context& context = GLRageGetContext();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    TRACEF("DirectDraw DllMain(%p,%d)", hModule, dwReason);
    return TRUE;
}

HRESULT WINAPI DirectDrawCreate(
    GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter)
{
    TRACEF("DirectDrawCreate");

    context.init();
    context.attach();

    ErrorUtils::setHWnd(context.getHWnd());

    try {
        *lplpDD = new DirectDraw();
    } catch (const std::exception& ex) {
        ErrorUtils::warning(ex);
        return DDERR_GENERIC;
    }

    return DD_OK;
}