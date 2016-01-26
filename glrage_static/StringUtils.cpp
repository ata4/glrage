#include "StringUtils.hpp"

#include <iomanip>
#include <sstream>
#include <locale>
#include <codecvt>

namespace glrage {

void StringUtils::format(std::string& str, const std::string& fmt, const int maxlen, ...) {
    va_list vl;
    va_start(vl, maxlen);
    formatImpl(str, fmt, maxlen, vl);
    va_end(vl);
}

std::string StringUtils::format(const std::string& fmt, const int maxlen, ...) {
    std::string result;
    va_list vl;
    va_start(vl, maxlen);
    formatImpl(result, fmt, maxlen, vl);
    va_end(vl);
    return result;
}

void StringUtils::formatImpl(std::string& str, const std::string& fmt, const int maxlen, va_list& vl) {
    str.reserve(maxlen);
    vsnprintf_s(&str[0], maxlen, _TRUNCATE, fmt.c_str(), vl);
}

std::string StringUtils::bytesToHex(const std::vector<uint8_t>& data) {
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    for (uint32_t byte : data) {
        ss <<  std::setw(2) << byte << ' ';
    }

    // remove last space
    std::string str = ss.str();
    if (!str.empty()) {
        str.pop_back();
    }

    return str;
}

std::vector<uint8_t> StringUtils::hexToBytes(const std::string& str) {
    std::vector<uint8_t> data;
    std::istringstream ss(str);
    ss >> std::hex >> std::setw(2);

    for (uint32_t byte; ss >> byte; ) {
        data.push_back(byte);
    }

    return data;
}

std::wstring StringUtils::utf8ToWide(const std::string& str) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    return convert.from_bytes(str);
}

std::string StringUtils::wideToUtf8(const std::wstring& str) {
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    return convert.to_bytes(str);
}

}