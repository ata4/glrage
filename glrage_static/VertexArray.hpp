#pragma once

#include "gl_core_3_3.h"
#include "Object.hpp"

namespace glrage {

class VertexArray : public Object {
public:
    VertexArray();
    ~VertexArray();
    void bind();
    void attribute(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLsizei offset);
};

}