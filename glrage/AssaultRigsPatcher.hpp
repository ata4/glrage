#pragma once

#include "RuntimePatcher.hpp"

namespace glrage {

class AssaultRigsPatcher : public RuntimePatcher {
public:
    AssaultRigsPatcher();
    virtual bool applicable(const std::string& fileName);
    virtual void apply();
};

}