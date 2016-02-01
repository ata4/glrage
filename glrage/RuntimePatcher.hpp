#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "GameID.hpp"
#include "Config.hpp"
#include "RuntimeData.hpp"

namespace glrage {

class RuntimePatcher {
public:
    RuntimePatcher(const std::string& configName);
    static GameID patch();
    virtual GameID gameID() = 0;
    virtual bool applicable(const std::string& fileName) = 0;
    virtual void apply() = 0;

protected:
    bool patch(uint32_t addr, const std::string& expected, const std::string& replacement);
    bool patch(uint32_t addr, const std::string& expected, const RuntimeData& replacement);
    bool patch(uint32_t addr, const RuntimeData& expected, const RuntimeData& replacement);

    bool patch(uint32_t addr, const std::string& replacement);
    bool patch(uint32_t addr, const RuntimeData& replacement);

    void patchAddr(int32_t addrCall, const std::string& expected, void* func, uint8_t op);

    Config m_config;
    RuntimeData m_tmp;
};

}