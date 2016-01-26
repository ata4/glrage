#pragma once

#include "gl_core_3_3.h"
#include "Object.hpp"

#include <string>

namespace glrage {

class Shader : public Object {
public:
    Shader(GLenum shaderType);
    ~Shader();
    void bind();
    Shader& fromFile(const std::wstring& path);
    Shader& fromString(const std::string& program);
    std::string infoLog();
    bool compiled();
};

}