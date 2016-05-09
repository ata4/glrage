#pragma once

#include "Object.hpp"
#include "gl_core_3_3.h"

namespace glrage {

class Texture : public Object
{
public:
    Texture(GLenum target);
    ~Texture();
    void bind();
    GLenum target()
    {
        return m_target;
    }

private:
    GLenum m_target;
};

} // namespace glrage