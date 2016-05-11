#pragma once

#include <Windows.h>
#include <intrin.h>

#include <string>

#define LOG_INFO(...) Logger::log(_ReturnAddress(), __FUNCTION__, __VA_ARGS__)

#ifdef LOG_TRACE_ENABLED
#define LOG_TRACE(...) Logger::log(_ReturnAddress(), __FUNCTION__, __VA_ARGS__)
#else
#define LOG_TRACE(...)
#endif

class Logger
{
public:
    static void log(void* returnAddress, const std::string& function,
        std::string format, ...);

private:
    static std::string m_buffer1;
    static std::string m_buffer2;
};