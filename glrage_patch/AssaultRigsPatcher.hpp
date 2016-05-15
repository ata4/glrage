#pragma once

#include "RuntimePatch.hpp"

namespace glrage {

class AssaultRigsPatcher : public RuntimePatch
{
public:
    virtual void apply();
};

} // namespace glrage