#pragma once

#include <cstdint>
#include <string>

namespace glrage {

class Config
{
public:
    Config(const std::wstring& path, const std::string& section);
    std::string getString(
        const std::string& name, const std::string& defaultValue);
    int32_t getInt(const std::string& name, const int32_t defaultValue);
    float getFloat(const std::string& name, const float defaultValue);
    bool getBool(const std::string& name, const bool defaultValue);

private:
    std::wstring m_configPath;
    std::string m_section;
};

} // namespace glrage