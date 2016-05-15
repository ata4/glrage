#pragma once

#include "RuntimePatch.hpp"

namespace glrage {

class TombRaiderPatcher : public RuntimePatch
{
public:
    TombRaiderPatcher(bool ub);
    virtual GameID gameID();
    virtual void apply();

private:
    void applyCrashPatches();
    void applyGraphicPatches();
    void applySoundPatches();
    void applyLogicPatches();
    void applyLocalePatches();

    bool m_ub;
};

} // namespace glrage