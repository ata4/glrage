#pragma once

#include "RuntimePatch.hpp"

namespace glrage {

class AssaultRigsPatcher : public RuntimePatch
{
public:
    virtual GameID gameID();
    virtual void apply();
};

} // namespace glrage