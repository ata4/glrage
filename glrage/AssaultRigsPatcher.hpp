#pragma once

#include "MemoryPatcher.hpp"

namespace glrage {

class AssaultRigsPatcher : public MemoryPatcher {
public:
    virtual bool applicable(const std::string& fileName);
    virtual void apply();
};

}