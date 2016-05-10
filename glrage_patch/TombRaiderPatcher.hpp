#pragma once

#include "RuntimePatcher.hpp"

namespace glrage {

class TombRaiderPatcher : public RuntimePatcher
{
public:
    TombRaiderPatcher(bool ub);
    virtual void apply(Config& config);

private:
    void applyCrashPatches(Config& config);
    void applyGraphicPatches(Config& config);
    void applySoundPatches(Config& config);
    void applyLogicPatches(Config& config);
    void applyLocalePatches(Config& config);

    bool m_ub;
};

} // namespace glrage