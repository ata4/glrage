#include "GLRage.hpp"

#include <glrage_util/Logger.hpp>
#include <glrage_patch/RuntimePatcherMain.hpp>

namespace glrage {

static ContextImpl context;
static RuntimePatcherMain patcher;

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    LOG_TRACE("%p,%d", hInst, dwReason);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            patcher.patch();
            break;
    }

    return TRUE;
}

Context& GLRageGetContextStatic()
{
    return context;
}

GLRAPI Context& GLRageGetContext()
{
    return context;
}

} // namespace glrage