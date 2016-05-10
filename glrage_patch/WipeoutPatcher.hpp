#pragma once

#include "RuntimePatcher.hpp"

#include <Windows.h>

namespace glrage {

class WipeoutPatcher : public RuntimePatcher
{
public:
    virtual void apply(Config& config);

private:
    static BOOL hookSystemParametersInfoA(
        UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
};

} // namespace glrage