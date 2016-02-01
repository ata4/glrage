#pragma once

#include "RuntimePatcher.hpp"

#include <Windows.h>

namespace glrage {

class WipeoutPatcher : public RuntimePatcher {
public:
    WipeoutPatcher();
    virtual GameID gameID();
    virtual bool applicable(const std::string& fileName);
    virtual void apply();

private:
    static BOOL hookSystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
};

}