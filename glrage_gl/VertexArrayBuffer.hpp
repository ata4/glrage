#pragma once

#include "VertexBuffer.hpp"

namespace glrage {

class VertexArrayBuffer : public VertexBuffer
{
public:
    VertexArrayBuffer()
        : VertexBuffer(GL_ARRAY_BUFFER){};
};

} // namespace glrage