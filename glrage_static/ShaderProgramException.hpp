#pragma once

#include <stdexcept>

namespace glrage {

class ShaderProgramException : public std::runtime_error
{
public:
    ShaderProgramException(const std::string& msg)
        : std::runtime_error(msg)
    {
    }
    ShaderProgramException(const char* msg)
        : std::runtime_error(msg)
    {
    }
};

} // namespace glrage