#pragma once

#include "gl_core_3_3.h"
#include "Object.hpp"

namespace glrage {

class VertexBuffer : public Object {
public:
    VertexBuffer(GLenum target);
    ~VertexBuffer();
    void bind();
    void data(GLsizei size, const void* data, GLenum usage);
    void subData(GLsizei offset, GLsizei size, const void* data);
    void* map(GLenum access);
    void unmap();
    GLint parameter(GLenum pname);

private:
    GLenum m_target;
};

}