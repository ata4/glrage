#pragma once

#include "RuntimePatcher.hpp"

namespace glrage {

class TombRaiderPatcher : public RuntimePatcher
{
public:
    TombRaiderPatcher();
    virtual GameID gameID();
    virtual bool applicable(const std::string& fileName);
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