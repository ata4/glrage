#pragma once

#include "Shader.hpp"

namespace glrage {

class VertexShader : public Shader {
public:
    VertexShader() : Shader(GL_VERTEX_SHADER) {};
};

}