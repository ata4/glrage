#pragma once

#include "ContextImpl.hpp"

#include <glrage_patch/RuntimePatcherMain.hpp>

namespace glrage {

#ifdef _LIB
#define GLRAPI
#elif defined(GLR_EXPORTS)
#define GLRAPI __declspec(dllexport)
#else
#define GLRAPI __declspec(dllimport)
#endif

class GLRage {
public:
    static GLRAPI Context& getContext();
    static GLRAPI RuntimePatcherMain& getPatcher();

private:
    static ContextImpl m_context;
    static RuntimePatcherMain m_patcher;
};

} // namespace glrage