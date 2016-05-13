#include "Shader.hpp"

namespace glrage {
namespace gl {

class FragmentShader : public Shader
{
public:
    FragmentShader()
        : Shader(GL_FRAGMENT_SHADER){};
};

} // namespace gl
} // namespace glrage