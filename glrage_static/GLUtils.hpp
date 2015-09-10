#pragma once

#include "gl_core_3_3.h"

namespace glrage {

class GLUtils {
public:
    static const char* getErrorString(GLenum);
    static void checkError(char*);
};

}