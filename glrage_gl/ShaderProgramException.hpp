#pragma once

#include <stdexcept>

namespace glrage {
namespace gl {

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

} // namespace gl
} // namespace glrage