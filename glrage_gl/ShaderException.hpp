#pragma once

#include <stdexcept>

namespace glrage {

class ShaderException : public std::runtime_error
{
public:
    ShaderException(const std::string& msg)
        : std::runtime_error(msg)
    {
    }
    ShaderException(const char* msg)
        : std::runtime_error(msg)
    {
    }
};

} // namespace glrage