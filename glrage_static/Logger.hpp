#pragma once

#include <Windows.h>
#include <intrin.h>
#include <string>

#ifdef DEBUG_LOG
#define LOG(...) Logger::log(_ReturnAddress(), __VA_ARGS__)
#define LOGF(...) Logger::logf(_ReturnAddress(), __VA_ARGS__)
#else
#define LOG(...)
#define LOGF(...)
#endif

#ifdef DEBUG_TRACE
#define TRACE(...) Logger::log(_ReturnAddress(), __VA_ARGS__)
#define TRACEF(...) Logger::logf(_ReturnAddress(), __VA_ARGS__)
#else
#define TRACE(...)
#define TRACEF(...)
#endif

class Logger {
public:
    static void log(void* returnAddress, const std::string& message);
    static void log(void* returnAddress, const char* message);
    static void logf(void* returnAddress, const char* message, ...);
private:
    static void logImpl(void* returnAddress, const char*);
    static const bool VERBOSE = true;
    static TCHAR m_name[MAX_PATH];
    static TCHAR m_info[1024];
    static TCHAR m_line[1024];
};