#pragma once

#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>

namespace glrage {

class StringUtils
{
public:
    static void format(
        std::string& str, const std::string& fmt, const int maxlen, ...);
    static std::string format(const std::string& fmt, const int maxlen, ...);
    static std::string bytesToHex(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> hexToBytes(const std::string& str);
    static std::wstring utf8ToWide(const std::string& str);
    static std::string wideToUtf8(const std::wstring& str);

private:
    static void formatImpl(std::string& str, const std::string& fmt,
        const int maxlen, va_list& va);
};

} // namespace glrage