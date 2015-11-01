#pragma once

#include "MemoryPatcher.hpp"

namespace glrage {

class TombRaiderPatcher : public MemoryPatcher {
public:
    TombRaiderPatcher();
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

}