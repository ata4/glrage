#include "Shader.hpp"

namespace glrage {

class FragmentShader : public Shader {
public:
    FragmentShader() : Shader(GL_FRAGMENT_SHADER) {};
};

}