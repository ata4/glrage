#include "Logger.hpp"

#include <string>
#include <iostream>

// fix for "K32GetModuleBaseNameA not found in KERNEL32.dll" error on Vista
#define PSAPI_VERSION 1
#include <psapi.h>

bool Logger::m_verbose = false;
LogTarget Logger::m_target = Win32Debug;

void Logger::setVerbose(bool verbose) {
    m_verbose = verbose;
}

bool Logger::isVerbose() {
    return m_verbose;
}

void Logger::setTarget(LogTarget target) {
    m_target = target;
}

LogTarget Logger::getTarget() {
    return m_target;
}

void Logger::log(void* returnAddress, const std::string& message) {
    log(returnAddress, message.c_str());
}

void Logger::log(void* returnAddress, const char* message) {
    logImpl(returnAddress, message);
}

void Logger::logf(void* returnAddress, const char* message, ...) {
    static char info[1024];

    va_list list;
    va_start(list, message);
    vsnprintf_s(info, sizeof(info), _TRUNCATE, message, list);
    va_end(list);

    logImpl(returnAddress, info);
}

void Logger::logImpl(void* returnAddress, const char* message) {
    static char line[1024];
    static char name[MAX_PATH];

    if (m_verbose) {
        HMODULE handle;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            static_cast<LPCTSTR>(returnAddress), &handle);
        GetModuleBaseNameA(GetCurrentProcess(), handle, name, sizeof(name));

        _snprintf_s(line, sizeof(line), _TRUNCATE, "[%s %p@%d] %s\n", name,
            returnAddress, GetCurrentThreadId(), message);
    } else {
        _snprintf_s(line, sizeof(line), _TRUNCATE, "%s\n", message);
    }

    switch (m_target) {
        case Console:
            std::cout << line;
            break;

        case Win32Debug:
            OutputDebugStringA(line);
            break;
    }
}