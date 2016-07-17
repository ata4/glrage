#include "Logger.hpp"

#include <cstdio>
#include <string>

static const size_t bufferSize = 1024;

void Logger::printf(const char* format, ...)
{
    char output[bufferSize];
    va_list list;
    va_start(list, format);
    vsnprintf_s(output, sizeof(output), _TRUNCATE, &format[0], list);
    va_end(list);

    OutputDebugStringA(output);
}

void Logger::printf(const std::string& msg)
{
    printf(msg.c_str());
}

void Logger::tracef(void* returnAddress, const char* function, const char* format, ...)
{
    if (strlen(format) == 0) {
        printf("%p %s\n", returnAddress, function);
        return;
    }

    char output[bufferSize];
    va_list list;
    va_start(list, format);
    vsnprintf_s(output, sizeof(output), _TRUNCATE, format, list);
    va_end(list);

    printf("%p %s: %s\n", returnAddress, function, output);
}

void Logger::tracef(void* returnAddress, const char* function, const std::string& msg)
{
    tracef(returnAddress, function, msg.c_str());
}