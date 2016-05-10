#include "Config.hpp"
#include "StringUtils.hpp"

#include <Windows.h>

namespace glrage {

Config::Config(const std::string& section, const std::wstring& directory)
{
    m_section = section;
    m_configPath = directory + L"\\glrage.ini";
}

std::string Config::getString(
    const std::string& name, const std::string& defaultValue)
{
    std::wstring sectionW = StringUtils::utf8ToWide(m_section);
    std::wstring nameW = StringUtils::utf8ToWide(name);
    std::wstring defaultValueW = StringUtils::utf8ToWide(defaultValue);
    static TCHAR valueW[1024];

    GetPrivateProfileString(sectionW.c_str(), nameW.c_str(),
        defaultValueW.c_str(), valueW, sizeof(valueW) / sizeof(valueW[0]),
        m_configPath.c_str());

    return StringUtils::wideToUtf8(valueW);
}

int32_t Config::getInt(const std::string& name, const int32_t defaultValue)
{
    return std::stoi(getString(name, std::to_string(defaultValue)));
}

float Config::getFloat(const std::string& name, const float defaultValue)
{
    return std::stof(getString(name, std::to_string(defaultValue)));
}

bool Config::getBool(const std::string& name, const bool defaultValue)
{
    return getString(name, defaultValue ? "true" : "false") == "true";
}

} // namespace glrage