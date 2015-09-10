#pragma once

#include "ddraw.hpp"
#include "VertexArray.hpp"
#include "VertexArrayBuffer.hpp"
#include "Texture.hpp"
#include "ShaderProgram.hpp"

#include <cstdint>

using glrage::VertexArray;
using glrage::VertexArrayBuffer;
using glrage::Texture;
using glrage::ShaderProgram;

namespace ddraw {

class SurfaceRenderer {
public:
    SurfaceRenderer();
    void upload(DDSURFACEDESC& desc, void* data);
    void render();
private:
    static const GLenum TEX_INTERNAL_FORMAT = GL_RGBA;
    static const GLenum TEX_FORMAT = GL_BGRA;
    static const GLenum TEX_TYPE = GL_UNSIGNED_SHORT_1_5_5_5_REV;
    uint32_t m_width;
    uint32_t m_height;
    VertexArray m_surfaceFormat;
    VertexArrayBuffer m_surfaceBuffer;
    Texture m_surfaceTexture;
    ShaderProgram m_program;
};

}