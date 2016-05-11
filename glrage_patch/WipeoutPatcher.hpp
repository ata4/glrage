#pragma once

#include "RuntimePatcher.hpp"

#include <Windows.h>

namespace glrage {

class WipeoutPatcher : public RuntimePatcher
{
public:
    virtual void apply();

private:
    static BOOL WINAPI hookSystemParametersInfoA(
        UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
};

} // namespace glrage