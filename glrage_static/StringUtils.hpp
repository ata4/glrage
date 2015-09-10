#pragma once

#include <string>
#include <cstdarg>
#include <cstdint>
#include <vector>

namespace glrage {

class StringUtils {
public:
    static void format(std::string& str, const std::string& fmt, const int maxlen, ...);
    static std::string format(const std::string& fmt, const int maxlen, ...);
    static std::string bytesToHex(std::vector<uint8_t>& data);
    static std::vector<uint8_t> hexToBytes(const std::string& str);
private:
    static void formatImpl(std::string& str, const std::string& fmt, const int maxlen, va_list& va);
};

}