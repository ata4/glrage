#include "GLRage.hpp"
#include "Logger.hpp"
#include "RuntimePatcher.hpp"

using glrage::RuntimePatcher;
using glrage::ContextImpl;

static ContextImpl context;

const char* DLLMAIN_REASON[]{"DLL_PROCESS_DETACH", "DLL_PROCESS_ATTACH",
    "DLL_THREAD_ATTACH", "DLL_THREAD_DETACH"};

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    TRACEF("GLRage DllMain(%p,%s,%08X)", hInst, DLLMAIN_REASON[dwReason],
        lpReserved);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            context.setGameID(RuntimePatcher::patch());
            break;
    }

    return TRUE;
}

extern "C" {

GLRAPI Context& GLRageGetContext()
{
    return context;
}

} // extern "C"