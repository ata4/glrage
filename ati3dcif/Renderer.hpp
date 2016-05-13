#pragma once

#include "Texture.hpp"
#include "VertexStream.hpp"

#include <glrage\GLRage.hpp>
#include <glrage_util\Config.hpp>
#include <glrage_gl\Sampler.hpp>
#include <glrage_gl\Shader.hpp>
#include <glrage_gl\ShaderProgram.hpp>

#include <memory>
#include <map>

namespace glrage {
namespace cif {

// ATI3DCIF -> OpenGL mapping tables
static const GLenum GLCIF_DEPTH_MASK[] = {
    GL_FALSE, // C3D_EZMODE_OFF (ignore z)
    GL_FALSE, // C3D_EZMODE_TESTON (test z, but do not update the z buffer)
    GL_TRUE   // C3D_EZMODE_TESTON_WRITEZ (test z and update the z buffer)
};

static const GLenum GLCIF_DEPTH_FUNC[] = {
    GL_NEVER,    // C3D_EZCMP_NEVER
    GL_LESS,     // C3D_EZCMP_LESS
    GL_LEQUAL,   // C3D_EZCMP_LEQUAL
    GL_EQUAL,    // C3D_EZCMP_EQUAL
    GL_GEQUAL,   // C3D_EZCMP_GEQUAL
    GL_GREATER,  // C3D_EZCMP_GREATER
    GL_NOTEQUAL, // C3D_EZCMP_NOTEQUAL
    GL_ALWAYS    // C3D_EZCMP_ALWAYS
};

static const GLenum GLCIF_BLEND_FUNC[] = {
    GL_ZERO,                // C3D_EASRC_ZERO / C3D_EADST_ZERO
    GL_ONE,                 // C3D_EASRC_ONE / C3D_EADST_ONE
    GL_DST_COLOR,           // C3D_EASRC_DSTCLR / C3D_EADST_DSTCLR
    GL_ONE_MINUS_DST_COLOR, // C3D_EASRC_INVDSTCLR / C3D_EADST_INVDSTCLR
    GL_SRC_ALPHA,           // C3D_EASRC_SRCALPHA / C3D_EADST_SRCALPHA
    GL_ONE_MINUS_SRC_ALPHA, // C3D_EASRC_INVSRCALPHA / C3D_EADST_INVSRCALPHA
    GL_DST_ALPHA,           // C3D_EASRC_DSTALPHA / C3D_EADST_DSTALPHA
    GL_ONE_MINUS_DST_ALPHA  // C3D_EASRC_INVDSTALPHA / C3D_EADST_INVDSTALPHA
};

static const GLenum GLCIF_TEXTURE_MIN_FILTER[] = {
    GL_NEAREST,               // C3D_ETFILT_MINPNT_MAGPNT
    GL_LINEAR,                // C3D_ETFILT_MINPNT_MAG2BY2
    GL_LINEAR,                // C3D_ETFILT_MIN2BY2_MAG2BY2
    GL_LINEAR_MIPMAP_LINEAR,  // C3D_ETFILT_MIPLIN_MAGPNT
    GL_LINEAR_MIPMAP_NEAREST, // C3D_ETFILT_MIPLIN_MAG2BY2
    GL_LINEAR_MIPMAP_LINEAR,  // C3D_ETFILT_MIPTRI_MAG2BY2
    GL_NEAREST                // C3D_ETFILT_MIN2BY2_MAGPNT
};

static const GLenum GLCIF_TEXTURE_MAG_FILTER[] = {
    GL_NEAREST, // C3D_ETFILT_MINPNT_MAGPNT (pick nearest texel (pnt) min/mag)
    GL_NEAREST, // C3D_ETFILT_MINPNT_MAG2BY2 (pnt min/bi-linear mag)
    GL_LINEAR,  // C3D_ETFILT_MIN2BY2_MAG2BY2 (2x2 blend min/bi-linear mag)
    GL_NEAREST, // C3D_ETFILT_MIPLIN_MAGPNT (1x1 blend min(between maps)/pick
                // nearest mag)
    GL_LINEAR,  // C3D_ETFILT_MIPLIN_MAG2BY2 (1x1 blend min(between
                // maps)/bi-linear mag)
    GL_LINEAR,  // C3D_ETFILT_MIPTRI_MAG2BY2 (Rage3: (2x2)x(2x2)(between
                // maps)/bi-linear mag)
    GL_LINEAR   // C3D_ETFILT_MIN2BY2_MAGPNT (Rage3:2x2 blend min/pick nearest
                // mag)
};

typedef std::map<C3D_HTX, std::shared_ptr<Texture>> TextureMap;

class Renderer
{
public:
    Renderer();
    void renderBegin(C3D_HRC);
    void renderEnd();
    void textureReg(C3D_PTMAP, C3D_PHTX);
    void textureUnreg(C3D_HTX);
    void texturePaletteCreate(C3D_ECI_TMAP_TYPE, void*, C3D_PHTXPAL);
    void texturePaletteDestroy(C3D_HTXPAL);
    void texturePaletteAnimate(
        C3D_HTXPAL, C3D_UINT32, C3D_UINT32, C3D_PPALETTENTRY);
    void renderPrimStrip(C3D_VSTRIP, C3D_UINT32);
    void renderPrimList(C3D_VLIST, C3D_UINT32);

    // state functions start
    void fogColor(C3D_COLOR color);
    void vertexType(C3D_EVERTEX type);
    void primType(C3D_EPRIM type);
    void solidColor(C3D_COLOR color);
    void shadeMode(C3D_ESHADE mode);
    void tmapEnable(C3D_BOOL enable);
    void tmapSelect(C3D_HTX handle);
    void tmapLight(C3D_ETLIGHT mode);
    void tmapPerspCor(C3D_ETPERSPCOR mode);
    void tmapFilter(C3D_ETEXFILTER filter);
    void tmapTexOp(C3D_ETEXOP op);
    void alphaSrc(C3D_EASRC func);
    void alphaDst(C3D_EADST func);
    void surfDrawPtr(C3D_PVOID ptr);
    void surfDrawPitch(C3D_UINT32 pitch);
    void surfDrawPixelFormat(C3D_EPIXFMT format);
    void surfVport(C3D_RECT vport);
    void fogEnable(C3D_BOOL enable);
    void ditherEnable(C3D_BOOL enable);
    void zCmpFunc(C3D_EZCMP func);
    void zMode(C3D_EZMODE mode);
    void surfZPtr(C3D_PVOID ptr);
    void surfZPitch(C3D_UINT32 pitch);
    void surfScissor(C3D_RECT scissor);
    void compositeEnable(C3D_BOOL enable);
    void compositeSelect(C3D_HTX select);
    void compositeFunc(C3D_ETEXCOMPFCN func);
    void compositeFactor(C3D_UINT32 fact);
    void compositeFilter(C3D_ETEXFILTER filter);
    void compositeFactorAlphaEnable(C3D_BOOL enable);
    void lodBiasLevel(C3D_UINT32 level);
    void alphaDstTestEnable(C3D_BOOL enable);
    void alphaDstTestFunc(C3D_EACMP func);
    void alphaDstWriteSelect(C3D_EASEL mode);
    void alphaDstReference(C3D_UINT32 ref);
    void specularEnable(C3D_BOOL enable);
    void enhancedColorRangeEnable(C3D_BOOL enable);
    // state functions end

private:
    Context& m_context = GLRage::getContext();
    Config m_config{m_context.getConfigPath(), "ATI3DCIF"};
    bool m_wireframe;
    TextureMap m_textures;
    C3D_PPALETTENTRY m_palette = nullptr;

    C3D_EASRC m_alphaSrc = C3D_EASRC_ONE;
    C3D_EADST m_alphaDst = C3D_EADST_ZERO;
    C3D_HTX m_tmap = 0;

    gl::ShaderProgram m_program;
    gl::Sampler m_sampler;
    VertexStream m_vertexStream;
};

} // namespace cif
} // namespace glrage