#pragma once

#include <stdexcept>

namespace glrage {

class ShaderException : public std::runtime_error {
public:
    ShaderException(std::string msg) : std::runtime_error(msg.c_str()){}
};

}