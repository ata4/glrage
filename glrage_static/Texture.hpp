#pragma once

#include "gl_core_3_3.h"
#include "Object.hpp"

namespace glrage {

class Texture : public Object {
public:
    Texture(GLenum target);
    ~Texture();
    void bind();
    GLenum target() {
        return m_target;
    }

private:
    GLenum m_target;
};

}