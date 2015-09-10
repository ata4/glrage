#pragma once

#include <string>
#include <cstdint>

namespace glrage {

class Config {
public:
    static std::string getString(const std::string& section, const std::string& name, const std::string& defaultValue);
    static int32_t getInt(const std::string& section, const std::string& name, const int32_t defaultValue);
    static bool getBool(const std::string& section, const std::string& name, const bool defaultValue);
private:
    static std::string configPath;
};

}