#pragma once

#include "Shader.hpp"

namespace glrage {
namespace gl {

class VertexShader : public Shader
{
public:
    VertexShader()
        : Shader(GL_VERTEX_SHADER){};
};

} // namespace gl
} // namespace glrage