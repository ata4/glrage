#pragma once

// declare DLLEXPORT if required
#ifndef DLLEXPORT
#define DLLEXPORT
#endif

// declare WINAPI if required
#ifndef WINAPI
#define WINAPI __stdcall
#endif

// make sure the API is exported, not imported
#define BUILD_AS_DLL

// include actual ATI3DCIF.H from 3D Rage SDK
#include <ragesdk/include/ATI3DCIF.H>