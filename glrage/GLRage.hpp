#pragma once

#include "ContextImpl.hpp"

namespace glrage {

#ifdef GLR_EXPORTS
#define GLRAPI extern "C" __declspec(dllexport)
#else
#define GLRAPI extern "C" __declspec(dllimport)
#endif

// for static libraries
Context& GLRageGetContextStatic();

// for dynamic libraries
GLRAPI Context& GLRageGetContext();

} // namespace glrage