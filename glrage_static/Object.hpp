#pragma once

#include "gl_core_3_3.h"

namespace glrage {

class Object
{
public:
    virtual ~Object(){};
    GLuint id()
    {
        return m_id;
    }
    virtual void bind() = 0;

protected:
    GLuint m_id = 0;
};

} // namespace glrage