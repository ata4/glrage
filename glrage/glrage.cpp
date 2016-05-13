#include "GLRage.hpp"

#include <glrage_util/Logger.hpp>

namespace glrage {

ContextImpl GLRage::m_context;
RuntimePatcherMain GLRage::m_patcher;

GLRAPI Context& GLRage::getContext()
{
    return m_context;
}

GLRAPI RuntimePatcherMain& GLRage::getPatcher()
{
    return m_patcher;
}

} // namespace glrage

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
    LOG_TRACE("%p,%d", hInst, dwReason);

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            glrage::GLRage::getPatcher().patch();
            break;
    }

    return TRUE;
}