#include "DirectDraw.hpp"

#include <glrage/GLRage.hpp>
#include <glrage_util/ErrorUtils.hpp>
#include <glrage_util/Logger.hpp>

#include <string>

namespace glrage {
namespace ddraw {

static Context& context = GLRage::getContext();

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    LOG_TRACE("%p,%d", hModule, dwReason);

    return TRUE;
}

HRESULT WINAPI DirectDrawCreate(
    GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter)
{
    LOG_TRACE("%p, %p, %p", lpGUID, lplpDD, pUnkOuter);

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

} // namespace ddraw
} // namespace glrage