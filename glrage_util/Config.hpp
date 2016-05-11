#pragma once

#include <cstdint>
#include <string>

namespace glrage {

class Config
{
public:
    Config(const std::wstring& path = L"", const std::string& section = "");

    std::wstring getPath();
    void setPath(const std::wstring& path);

    std::string getSection();
    void setSection(const std::string& section);

    std::string getString(
        const std::string& name, const std::string& defaultValue);
    int32_t getInt(const std::string& name, const int32_t defaultValue);
    float getFloat(const std::string& name, const float defaultValue);
    bool getBool(const std::string& name, const bool defaultValue);

private:
    std::wstring m_path;
    std::wstring m_section;
    std::wstring m_value;
};

} // namespace glrage