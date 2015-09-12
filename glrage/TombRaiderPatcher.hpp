#pragma once

#include "MemoryPatcher.hpp"
#include "Config.hpp"

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

    Config m_config;
    bool m_ub;
};

}