#pragma once

#include "ContextImpl.hpp"

using glrage::Context;

#ifdef GLR_EXPORTS
#define GLRAPI __declspec(dllexport)
#else
#define GLRAPI __declspec(dllimport)
#endif

extern "C" {

GLRAPI Context& GLRageGetContext();

}