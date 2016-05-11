#include "Logger.hpp"

#include <cstdio>
#include <string>

static const size_t bufferMax = 1024;

std::string Logger::m_buffer1;
std::string Logger::m_buffer2;

void Logger::log(void* returnAddress, const std::string& function,
        std::string format, ...)
{
    int size;

    if (format.empty()) {
        m_buffer1.resize(bufferMax);
        size = sprintf_s(&m_buffer1[0], m_buffer1.capacity(), "%p %s\n",
            returnAddress, function.c_str());
        m_buffer1.resize(size);
    } else {
        m_buffer1.resize(bufferMax);
        va_list list;
        va_start(list, format);
        size = vsnprintf_s(
            &m_buffer1[0], m_buffer1.capacity(), _TRUNCATE, &format[0], list);
        va_end(list);
        m_buffer1.resize(size);

        m_buffer2.resize(bufferMax);
        size = sprintf_s(&m_buffer2[0], m_buffer2.capacity(), "%p %s: %s\n",
            returnAddress, function.c_str(), m_buffer1.c_str());
        m_buffer2.resize(size);
    }
    

    OutputDebugStringA(m_buffer2.c_str());
}