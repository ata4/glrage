#include "RuntimePatcher.hpp"
#include "TombRaiderPatcher.hpp"
#include "AssaultRigsPatcher.hpp"
#include "WipeoutPatcher.hpp"

#include "StringUtils.hpp"
#include "Logger.hpp"
#include "GLRage.hpp"

#include <Windows.h>
#include <Shlwapi.h>

#include <algorithm>

namespace glrage {

template<class T> static void RuntimePatcher::runPatch(const std::string& fileName) {
    T patch = T();
    if (patch.applicable(fileName)) {
        patch.apply();
    }
}

RuntimePatcher::RuntimePatcher(const std::string& configName) :
    m_config(configName, GLRageGetContext().getBasePath()) {
}

void RuntimePatcher::patch() {
    

    // get executable name
    TCHAR modulePath[MAX_PATH];
    GetModuleFileName(nullptr, modulePath, MAX_PATH);

    // extract file name
    TCHAR* modulePathTmp = PathFindFileName(modulePath);

    // remove extension
    PathRemoveExtension(modulePathTmp);

    std::wstring modulePathW = modulePathTmp;

    // convert to lower case
    transform(modulePathW.begin(), modulePathW.end(), modulePathW.begin(), ::towlower);

    // convert to UTF-8
    std::string moduleFileName = std::string(StringUtils::wideToUtf8(modulePathW));

    // run known patches
    runPatch<TombRaiderPatcher>(moduleFileName);
    runPatch<AssaultRigsPatcher>(moduleFileName);
    runPatch<WipeoutPatcher>(moduleFileName);
}

bool RuntimePatcher::patch(uint32_t addr, const std::string& expected, const std::string& replacement) {
    RuntimeData expectedData(StringUtils::hexToBytes(expected));
    RuntimeData replacementData(StringUtils::hexToBytes(replacement));
    return patch(addr, expectedData, replacementData);
}

bool RuntimePatcher::patch(uint32_t addr, const std::string& expected, const RuntimeData& replacement) {
    RuntimeData expectedData(StringUtils::hexToBytes(expected));
    return patch(addr, expectedData, replacement);
}

bool RuntimePatcher::patch(uint32_t addr, const RuntimeData& expected, const RuntimeData& replacement) {
    bool result = false;
    bool restoreProtect = false;

    const std::vector<uint8_t> expectedData = expected.data();
    const std::vector<uint8_t> replacementData = replacement.data();

    size_t size = expectedData.size();
    std::vector<uint8_t> actualData(size);

    // vectors must match in size
    if (size != replacementData.size()) {
        goto end;
    }

    // apply read/write flags to the memory page
    DWORD oldProtect = 0;
    LPVOID lpaddr = reinterpret_cast<LPVOID>(addr);
    if (!VirtualProtect(lpaddr, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        goto end;
    }

    restoreProtect = true;

    // read current memory to a temporary vector
    HANDLE proc = GetCurrentProcess();
    DWORD numRead = 0;
    if (!ReadProcessMemory(proc, lpaddr, &actualData[0], size, &numRead) || numRead != size) {
        goto end;
    }

    // compare actual data with expected data
    if (actualData != expectedData) {
        goto end;
    }

    // write patched data to memory
    DWORD numWritten = 0;
    if (!WriteProcessMemory(proc, lpaddr, &replacementData[0], size, &numWritten) || numWritten != size) {
        goto end;
    }

    result = true;

    end:
    // restore original page flags
    if (restoreProtect) {
        VirtualProtect(lpaddr, size, oldProtect, nullptr);
    }

    if (!result) {
        LOGF("Patch at 0x%x with %d bytes failed!", addr, size);
        LOG("Expected: " + StringUtils::bytesToHex(expectedData));
        LOG("Actual:   " + StringUtils::bytesToHex(actualData));
        LOG("Patch:    " + StringUtils::bytesToHex(replacementData));
    }

    return result;
}

void RuntimePatcher::patchAddr(int32_t addrCall, const std::string& expected, void* func, uint8_t op) {
    int32_t addrFunc = reinterpret_cast<int32_t>(func);
    int32_t addrCallNew = addrFunc - addrCall - 5;

    m_tmp.clear();
    m_tmp << op << addrCallNew;

    patch(addrCall, expected, m_tmp);
}

}