#include "Config.hpp"
#include "StringUtils.hpp"

#include <Windows.h>

namespace glrage {

Config::Config(const std::wstring& path, const std::string& section)
{
    setPath(path);
    setSection(section);
}

std::wstring Config::getPath()
{
    return m_path;
}

void Config::setPath(const std::wstring& path)
{
    m_path = path;
}

std::string Config::getSection()
{
    return StringUtils::wideToUtf8(m_section);
}

void Config::setSection(const std::string& section)
{
    m_section = StringUtils::utf8ToWide(section);
}

std::string Config::getString(
    const std::string& name, const std::string& defaultValue)
{
    std::wstring nameW = StringUtils::utf8ToWide(name);
    std::wstring defaultValueW = StringUtils::utf8ToWide(defaultValue);

    m_value.resize(1024);
    DWORD size = GetPrivateProfileString(m_section.c_str(), nameW.c_str(),
        defaultValueW.c_str(), &m_value[0], m_value.capacity(),
        m_path.c_str());
    m_value.resize(size);

    return StringUtils::wideToUtf8(m_value);
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