#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Config.hpp"
#include "RuntimeData.hpp"

namespace glrage {

class RuntimePatcher {
public:
    RuntimePatcher(const std::string& configName);
    static void patch();
    virtual bool applicable(const std::string& fileName) = 0;
    virtual void apply() = 0;

protected:
    template<class T>
    static void runPatch(const std::string& fileName);
    bool patch(uint32_t addr, const std::string& expected, const std::string& replacement);
    bool patch(uint32_t addr, const std::string& expected, const RuntimeData& replacement);
    bool patch(uint32_t addr, const RuntimeData& expected, const RuntimeData& replacement);
    void patchAddr(int32_t addrCall, const std::string& expected, void* func, uint8_t op);

    Config m_config;
    RuntimeData m_tmp;
};

}