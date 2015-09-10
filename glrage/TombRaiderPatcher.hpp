#pragma once

#include "MemoryPatcher.hpp"

namespace glrage {

class TombRaiderPatcher : public MemoryPatcher {
public:
    virtual bool applicable(const std::string& fileName);
    virtual void apply();
private:
    void applyCrashPatches();
    void applyGraphicPatches();
    void applySoundPatches();
    void applyLogicPatches();

    bool m_ub;
};

}