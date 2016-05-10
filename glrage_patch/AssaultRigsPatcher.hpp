#pragma once

#include "RuntimePatcher.hpp"

namespace glrage {

class AssaultRigsPatcher : public RuntimePatcher
{
public:
    virtual void apply(Config& config);
};

} // namespace glrage