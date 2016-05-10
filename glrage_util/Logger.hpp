#pragma once

#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <string>

#ifdef DEBUG_LOG
#define LOG(msg) Logger::log(_ReturnAddress(), msg)
#define LOGF(...) Logger::logf(_ReturnAddress(), __VA_ARGS__)
#else
#define LOG(msg)
#define LOGF(...)
#endif

#ifdef DEBUG_TRACE
#define TRACE(msg) LOG(msg)
#define TRACEF(...) LOGF(__VA_ARGS__)
#else
#define TRACE(msg)
#define TRACEF(...)
#endif

enum LogTarget
{
    Console,
    Win32Debug
};

class Logger
{
public:
    static void setVerbose(bool verbose);
    static bool isVerbose();
    static void setTarget(LogTarget target);
    static LogTarget getTarget();
    static void log(void* returnAddress, const std::string& message);
    static void log(void* returnAddress, const char* message);
    static void logf(void* returnAddress, const char* message, ...);

private:
    static void logImpl(void* returnAddress, const char*);
    static bool m_verbose;
    static LogTarget m_target;
    // static char m_name[MAX_PATH];
    // static TCHAR m_namew[MAX_PATH];
    // static char m_info[1024];
    // static char m_line[1024];
};