#include "RuntimePatcher.hpp"
#include "TombRaiderPatcher.hpp"
#include "AssaultRigsPatcher.hpp"

#include "StringUtils.hpp"
#include "Logger.hpp"

#include <Windows.h>
#include <Shlwapi.h>
#include <algorithm>

namespace glrage {

template<class T> void RuntimePatcher::appendBytes(T value, std::vector<uint8_t>& data) {
    auto valueBytes = reinterpret_cast<uint8_t const*>(&value);
    for (size_t i = 0; i < sizeof(T); i++) {
        data.push_back(valueBytes[i]);
    }
}

template void RuntimePatcher::appendBytes<int32_t>(int32_t, std::vector<uint8_t>&);
template void RuntimePatcher::appendBytes<int16_t>(int16_t, std::vector<uint8_t>&);
template void RuntimePatcher::appendBytes<float_t>(float_t, std::vector<uint8_t>&);

template<class T> static void RuntimePatcher::runPatch(const std::string& fileName) {
    T patch = T();
    if (patch.applicable(fileName)) {
        patch.apply();
    }
}

RuntimePatcher::RuntimePatcher(const std::string& configName) :
    m_config(configName) {
}

void RuntimePatcher::patch() {
    // get executable name
    TCHAR modulePath[MAX_PATH] = { 0 };
    GetModuleFileName(nullptr, modulePath, MAX_PATH);

    // extract file name from path
    std::string moduleFileName = std::string(PathFindFileName(modulePath));

    // remove extension
    int32_t extIndex = moduleFileName.find_last_of(".");
    if (extIndex) {
        moduleFileName = moduleFileName.substr(0, extIndex);
    }

    // convert to lower case
    transform(moduleFileName.begin(), moduleFileName.end(), moduleFileName.begin(), ::tolower);

    // run known patches
    runPatch<TombRaiderPatcher>(moduleFileName);
    runPatch<AssaultRigsPatcher>(moduleFileName);
}

bool RuntimePatcher::patch(uint32_t addr, const std::string& expected, const std::string& replacement) {
    // parse strings to byte vectors
    std::vector<uint8_t> originalData = StringUtils::hexToBytes(expected);
    std::vector<uint8_t> replacementData = StringUtils::hexToBytes(replacement);

    return patch(addr, originalData, replacementData);
}

bool RuntimePatcher::patch(uint32_t addr, const std::string& expected, std::vector<uint8_t>& replacementData) {
    // parse strings to byte vectors
    std::vector<uint8_t> originalData = StringUtils::hexToBytes(expected);

    return patch(addr, originalData, replacementData);
}

bool RuntimePatcher::patch(uint32_t addr, std::vector<uint8_t>& expectedData, std::vector<uint8_t>& replacementData) {
    bool result = false;
    bool restoreProtect = false;
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

    LOGF("Patch at 0x%x with %d bytes %s", addr, size, result ? "successful" : "failed");
    LOG("Expected: " + StringUtils::bytesToHex(expectedData));
    
    if (!result) {
        LOG("Actual:   " + StringUtils::bytesToHex(actualData));
    }

    LOG("Patched:  " + StringUtils::bytesToHex(replacementData));

    return result;
}

void RuntimePatcher::patchAddr(int32_t addrCall, const std::string& expected, void* func, uint8_t op) {
    int32_t addrFunc = reinterpret_cast<int32_t>(func);
    int32_t addrCallNew = addrFunc - addrCall - 5;

    std::vector<uint8_t> replacement;
    replacement.push_back(op);
    appendBytes(addrCallNew, replacement);

    patch(addrCall, expected, replacement);
}

}