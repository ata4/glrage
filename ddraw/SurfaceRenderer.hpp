#pragma once

#include "ddraw.hpp"
#include "VertexArray.hpp"
#include "VertexArrayBuffer.hpp"
#include "Texture.hpp"
#include "Sampler.hpp"
#include "ShaderProgram.hpp"
#include "Config.hpp"

#include <cstdint>
#include <vector>

using glrage::VertexArray;
using glrage::VertexArrayBuffer;
using glrage::Texture;
using glrage::ShaderProgram;
using glrage::Sampler;
using glrage::Config;

namespace ddraw {

class SurfaceRenderer {
public:
    SurfaceRenderer();
    void upload(DDSURFACEDESC& desc, std::vector<uint8_t>& data);
    void render();

private:
    static const GLenum TEX_INTERNAL_FORMAT = GL_RGBA;
    static const GLenum TEX_FORMAT = GL_BGRA;
    static const GLenum TEX_TYPE = GL_UNSIGNED_SHORT_1_5_5_5_REV;

    Config m_config{ "DirectDraw" };
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    VertexArray m_surfaceFormat;
    VertexArrayBuffer m_surfaceBuffer;
    Texture m_surfaceTexture = GL_TEXTURE_2D;
    Sampler m_sampler;
    ShaderProgram m_program;
};

}