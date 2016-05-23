#pragma once

#include "Patch.hpp"
#include "RuntimePatcher.hpp"

#include <string>
#include <array>

namespace glrage {

class RuntimePatcher
{
public:
    void patch();

private:
    void getModulePath();
    void getModuleVersion();

    ModuleContext m_ctx;
};

} // namespace glrage