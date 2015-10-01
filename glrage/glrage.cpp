#include "GLRage.hpp"
#include "MemoryPatcher.hpp"
#include "Logger.hpp"

using glrage::MemoryPatcher;
using glrage::ContextImpl;

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

static ContextImpl context;

extern "C" {

GLRAPI Context& GLRageGetContext() {
    return context;
}

}