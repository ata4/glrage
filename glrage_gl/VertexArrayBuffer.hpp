#pragma once

#include "VertexBuffer.hpp"

namespace glrage {
namespace gl {

class VertexArrayBuffer : public VertexBuffer
{
public:
    VertexArrayBuffer()
        : VertexBuffer(GL_ARRAY_BUFFER){};
};

} // namespace gl
} // namespace glrage