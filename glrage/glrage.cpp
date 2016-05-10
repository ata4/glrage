#include "GLRage.hpp"
#include "Logger.hpp"
#include "RuntimePatcher.hpp"

using glrage::RuntimePatcher;
using glrage::ContextImpl;

static ContextImpl context;

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    TRACEF("GLRage DllMain(%p,%d)", hModule, dwReason);
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            RuntimePatcher::patch();
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