#pragma once

#include <string>
#include <cstdint>

namespace glrage {

class Config {
public:
    Config(const std::string& section);
    std::string getString(const std::string& name, const std::string& defaultValue);
    int32_t getInt(const std::string& name, const int32_t defaultValue);
    float getFloat(const std::string& name, const float defaultValue);
    bool getBool(const std::string& name, const bool defaultValue);

private:
    static std::string m_configPath;
    std::string m_section;
};

}