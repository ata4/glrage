#pragma once

#include "ddraw.hpp"

#include <string>

namespace ddraw {

class SurfaceDebugUtils
{
public:
    static void dumpInfo(DDSURFACEDESC& desc);
    static void dumpBuffer(
        DDSURFACEDESC& desc, void* buffer, const std::wstring& path);
    static std::string getSurfaceName(DDSURFACEDESC& desc);
};

} // namespace ddraw