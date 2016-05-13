#include "VertexBuffer.hpp"

namespace glrage {
namespace gl {

VertexBuffer::VertexBuffer(GLenum target)
    : m_target(target)
{
    glGenBuffers(1, &m_id);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_id);
}

void VertexBuffer::bind()
{
    glBindBuffer(m_target, m_id);
}

void VertexBuffer::data(GLsizei size, const void* data, GLenum usage)
{
    glBufferData(m_target, size, data, usage);
}

void VertexBuffer::subData(GLsizei offset, GLsizei size, const void* data)
{
    glBufferSubData(m_target, offset, size, data);
}

void* VertexBuffer::map(GLenum access)
{
    return glMapBuffer(m_target, access);
}

void VertexBuffer::unmap()
{
    glUnmapBuffer(m_target);
}

GLint VertexBuffer::parameter(GLenum pname)
{
    GLint params = 0;
    glGetBufferParameteriv(m_target, pname, &params);
    return params;
}

} // namespace gl
} // namespace glrage