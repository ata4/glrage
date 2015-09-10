#include "StringUtils.hpp"

#include <iomanip>
#include <sstream>

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

std::string StringUtils::bytesToHex(std::vector<uint8_t>& data) {
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (int c : data) {
        ss << std::setw(2) << c;
        ss << ' ';
    }
    std::string str = ss.str();
    if (!str.empty()) {
        str.pop_back();
    }
    return str;
}

std::vector<uint8_t> StringUtils::hexToBytes(const std::string& str) {
    std::vector<uint8_t> data;
    size_t len = str.length();
    for (size_t i = 0; i < len; i += 3) {
        std::string hex = str.substr(i, 2);
        int32_t dec = std::stoi(hex, nullptr, 16);
        data.push_back(dec);
    }
    return data;
}

}