#pragma once

#include "Buffer.hpp"

namespace glrage {
namespace gl {

class ArrayBuffer : public Buffer
{
public:
    ArrayBuffer()
        : Buffer(GL_ARRAY_BUFFER){};
};

} // namespace gl
} // namespace glrage