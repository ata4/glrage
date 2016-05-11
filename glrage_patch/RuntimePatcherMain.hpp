#pragma once

#include "RuntimePatcher.hpp"

#include <string>
#include <array>

namespace glrage {

class RuntimePatcherMain
{
public:
    void patch();

private:
    void getModulePath();
    void getModuleVersion();

    ModuleContext m_ctx;
};

} // namespace glrage