#pragma once

#include "RuntimeData.hpp"

#include <glrage/GameID.hpp>
#include <glrage_util/Config.hpp>

#include <Windows.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace glrage {

struct ModuleContext
{
    Config config{L"", "Patch"};
    std::wstring path;
    std::string fileName;
    std::wstring fileNameW;
    VS_FIXEDFILEINFO fileInfo;
};

class RuntimePatch
{
public:
    virtual void apply() = 0;
    virtual GameID gameID();
    void setContext(ModuleContext& ctx);

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
    ModuleContext m_ctx;
};

} // namespace glrage