#pragma once

#include "MemoryPatcher.hpp"

namespace glrage {

class AssaultRigsPatcher : public MemoryPatcher {
public:
    AssaultRigsPatcher();
    virtual bool applicable(const std::string& fileName);
    virtual void apply();
};

}