#include "Config.hpp"

#include <Windows.h>

namespace glrage {

std::string Config::configPath = ".\\glrage.ini";

std::string Config::getString(const std::string& section, const std::string& name, const std::string& defaultValue) {
    static TCHAR value[1024];
    GetPrivateProfileString(section.c_str(), name.c_str(), defaultValue.c_str(),
        value, sizeof(value) / sizeof(value[0]), configPath.c_str());
    return std::string(value);
}

int32_t Config::getInt(const std::string& section, const std::string& name, const int32_t defaultValue) {
    return GetPrivateProfileInt(section.c_str(), name.c_str(), defaultValue, configPath.c_str());
}

bool Config::getBool(const std::string& section, const std::string& name, const bool defaultValue) {
    return getString(section, name, defaultValue ? "true" : "false") == "true";
}

}