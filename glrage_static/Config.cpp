#include "Config.hpp"

#include <Windows.h>

namespace glrage {

std::string Config::m_configPath = ".\\glrage.ini";

Config::Config(const std::string& section) {
    m_section = section;
}

std::string Config::getString(const std::string& name, const std::string& defaultValue) {
    static TCHAR value[1024];
    GetPrivateProfileString(m_section.c_str(), name.c_str(), defaultValue.c_str(),
        value, sizeof(value) / sizeof(value[0]), m_configPath.c_str());
    return std::string(value);
}

int32_t Config::getInt(const std::string& name, const int32_t defaultValue) {
    return std::stoi(getString(name, std::to_string(defaultValue)));
}

float Config::getFloat(const std::string& name, const float defaultValue) {
    return std::stof(getString(name, std::to_string(defaultValue)));
}

bool Config::getBool(const std::string& name, const bool defaultValue) {
    return getString(name, defaultValue ? "true" : "false") == "true";
}

}