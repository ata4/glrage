#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Config.hpp"
#include "RuntimeData.hpp"

namespace glrage {

class RuntimePatcher
{
public:
    static void patch();
    virtual void apply(Config& config) = 0;

protected:
    bool patch(uint32_t addr, const std::string& expected,
        const std::string& replacement);
    bool patch(uint32_t addr, const std::string& expected,
        const RuntimeData& replacement);
    bool patch(uint32_t addr, const RuntimeData& expected,
        const RuntimeData& replacement);

    bool patch(uint32_t addr, const std::string& replacement);
    bool patch(uint32_t addr, const RuntimeData& replacement);

    void patchAddr(
        int32_t addrCall, const std::string& expected, void* func, uint8_t op);

    bool patchNop(uint32_t addr, const std::string& expected);

    RuntimeData m_tmp;
};

} // namespace glrage