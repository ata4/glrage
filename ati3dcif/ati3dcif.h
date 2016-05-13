#pragma once

// declare DLLEXPORT if required
#ifndef DLLEXPORT
#define DLLEXPORT
#endif

// declare WINAPI if required
#ifndef WINAPI
#define WINAPI __stdcall
#endif

// export macro for CIF implementation
#ifdef BUILD_AS_DLL
#define EXPORT(function_name, return_type, parameters)      \
function_name##_t function_name##_lib = function_name;      \
return_type WINAPI function_name parameters
#endif

// include actual ATI3DCIF.H from 3D Rage SDK
#include <ragesdk\include\ATI3DCIF.H>