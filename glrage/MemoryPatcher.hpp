#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace glrage {

class MemoryPatcher {
public:
    static void patch();
    virtual bool applicable(const std::string& fileName) = 0;
    virtual void apply() = 0;
protected:
    template<class T> static void runPatch(const std::string& fileName);
    bool patch(uint32_t addr, const std::string& expected, const std::string& replacement);
    bool patch(uint32_t addr, const std::string& expected, std::vector<uint8_t>& replacement);
    bool patch(uint32_t addr, std::vector<uint8_t>& expected, std::vector<uint8_t>& replacement);
    void patchAddr(int32_t addrCall, const std::string& expected, void* func, uint8_t op);
    template<class T> void appendBytes(T value, std::vector<uint8_t>& data);
};

}