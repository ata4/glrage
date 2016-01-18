#include "Logger.hpp"

#include <string>
#include <iostream>

// fix for "K32GetModuleBaseNameA not found in KERNEL32.dll" error on Vista
#define PSAPI_VERSION 1
#include <psapi.h>

bool Logger::m_verbose = false;
LogTarget Logger::m_target = Win32Debug;
TCHAR Logger::m_name[MAX_PATH];
TCHAR Logger::m_info[1024];
TCHAR Logger::m_line[1024];

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
    va_list list;
    va_start(list, message);
    vsnprintf_s(m_info, sizeof(m_info), _TRUNCATE, message, list);
    va_end(list);

    logImpl(returnAddress, m_info);
}

void Logger::logImpl(void* returnAddress, const char* message) {
    if (m_verbose) {
        HMODULE handle;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
            static_cast<LPCTSTR>(returnAddress), &handle);
        GetModuleBaseName(GetCurrentProcess(), handle, m_name, sizeof(m_name));
        _snprintf_s(m_line, sizeof(m_line), _TRUNCATE, "[%s %p@%d] %s\n", m_name,
            returnAddress, GetCurrentThreadId(), message);
    } else {
        _snprintf_s(m_line, sizeof(m_line), _TRUNCATE, "%s\n", message);
    }

    switch (m_target) {
        case Console:
            std::cout << m_line;
            break;

        case Win32Debug:
            OutputDebugString(m_line);
            break;
    }
}