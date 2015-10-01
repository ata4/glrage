#pragma once

#include "ContextImpl.hpp"

#include <Windows.h>
#include <cstdint>

using glrage::Context;

#ifdef GLR_EXPORTS
#define GLRAPI __declspec(dllexport)
#else
#define GLRAPI __declspec(dllimport)
#endif

extern "C" {

GLRAPI Context& GLRageGetContext();

}