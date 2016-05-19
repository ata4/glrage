#include "Renderer.hpp"
#include "Error.hpp"
#include "Utils.hpp"

#include <glrage_gl/Utils.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

// Disable VC "new behavior" warning regarding the array initializer
#pragma warning(disable : 4351)

namespace glrage {
namespace cif {

Renderer::Renderer()
{
    // bind sampler
    m_sampler.bind(0);

    // improve texture filtering quality
    float filterAniso = m_config.getFloat("filter_anisotropy", 16.0f);
    if (filterAniso > 0) {
        m_sampler.parameterf(GL_TEXTURE_MAX_ANISOTROPY_EXT, filterAniso);
    }

    // compile and link shaders and configure program
    std::wstring basePath = m_context.getBasePath();
    m_program.attach(gl::Shader(GL_VERTEX_SHADER)
                         .fromFile(basePath + L"\\shaders\\ati3dcif.vsh"));
    m_program.attach(gl::Shader(GL_FRAGMENT_SHADER)
                         .fromFile(basePath + L"\\shaders\\ati3dcif.fsh"));
    m_program.link();
    m_program.fragmentData("fragColor");
    m_program.bind();

    // negate Z axis so the model is rendered behind the viewport, which is
    // better
    // than having a negative zNear in the ortho matrix, which seems to mess up
    // depth testing
    glm::mat4 modelView = glm::scale(glm::mat4(), glm::vec3(1, 1, -1));
    m_program.uniformMatrix4fv(
        "matModelView", 1, GL_FALSE, glm::value_ptr(modelView));

    // cache frequently used config values
    m_wireframe = m_config.getBool("wireframe", false);

    gl::Utils::checkError(__FUNCTION__);
}

void Renderer::renderBegin(C3D_HRC hRC)
{
    glEnable(GL_BLEND);

    // set wireframe mode if set
    if (m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // bind objects
    m_program.bind();
    m_vertexStream.bind();
    m_sampler.bind(0);

    // restore texture binding
    tmapSelect(m_tmap);

    // CIF always uses an orthographic view, the application deals with the
    // perspective when required
    float width = static_cast<float>(m_context.getDisplayWidth());
    float height = static_cast<float>(m_context.getDisplayHeight());
    glm::mat4 projection = glm::ortho<float>(0, width, height, 0, -1e6, 1e6);
    m_program.uniformMatrix4fv(
        "matProjection", 1, GL_FALSE, glm::value_ptr(projection));

    gl::Utils::checkError(__FUNCTION__);
}

void Renderer::renderEnd()
{
    m_vertexStream.renderPending();

    // restore polygon mode
    if (m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Renderer::textureReg(C3D_PTMAP ptmapToReg, C3D_PHTX phtmap)
{
    // LOG_TRACE("fmt=%d, xlg2=%d, ylg2=%d, mip=%d",
    //    ptmapToReg->eTexFormat, ptmapToReg->u32MaxMapXSizeLg2,
    //    ptmapToReg->u32MaxMapYSizeLg2, ptmapToReg->bMipMap);

    std::shared_ptr<Texture> texture = std::make_shared<Texture>();
    texture->bind();
    texture->load(ptmapToReg, m_palette);

    // use id as texture handle
    *phtmap = reinterpret_cast<C3D_HTX>(texture->id());

    // store in texture map
    m_textures[*phtmap] = texture;

    gl::Utils::checkError(__FUNCTION__);
}

void Renderer::textureUnreg(C3D_HTX htxToUnreg)
{
    // LOG_TRACE("id=%d", id);

    TextureMap::iterator it = m_textures.find(htxToUnreg);
    if (it == m_textures.end()) {
        throw Error("Invalid texture handle", C3D_EC_BADPARAM);
    }

    // unbind texture if it's current
    if (htxToUnreg == m_tmap) {
        glBindTexture(GL_TEXTURE_2D, 0);
        m_tmap = 0;
    }

    std::shared_ptr<Texture> texture = it->second;
    m_textures.erase(htxToUnreg);
}

void Renderer::texturePaletteCreate(
    C3D_ECI_TMAP_TYPE epalette, void* pPalette, C3D_PHTXPAL phtpalCreated)
{
    switch (epalette) {
        case C3D_ECI_TMAP_8BIT: {
            m_palette = static_cast<C3D_PPALETTENTRY>(pPalette);
            break;
        }

        default:
            throw Error("Unsupported palette type: " +
                               std::string(C3D_ECI_TMAP_TYPE_NAMES[epalette]),
                C3D_EC_NOTIMPYET);
    }
}

void Renderer::texturePaletteDestroy(C3D_HTXPAL htxpalToDestroy)
{
    if (m_palette) {
        delete m_palette;
        m_palette = nullptr;
    }
}

void Renderer::texturePaletteAnimate(C3D_HTXPAL htxpalToAnimate,
    C3D_UINT32 u32StartIndex, C3D_UINT32 u32NumEntries,
    C3D_PPALETTENTRY pclrPalette)
{
    throw Error("Renderer::texturePaletteAnimate: Not implemented",
        C3D_EC_NOTIMPYET);
}

void Renderer::renderPrimStrip(C3D_VSTRIP vStrip, C3D_UINT32 u32NumVert)
{
    m_vertexStream.addPrimStrip(vStrip, u32NumVert);
}

void Renderer::renderPrimList(C3D_VLIST vList, C3D_UINT32 u32NumVert)
{
    m_vertexStream.addPrimList(vList, u32NumVert);
}

void Renderer::changeState()
{
    m_vertexStream.renderPending();
}

void Renderer::fogColor(C3D_COLOR color)
{
    // TODO
}

void Renderer::vertexType(C3D_EVERTEX type)
{
    m_vertexStream.vertexType(type);
}

void Renderer::primType(C3D_EPRIM type)
{
    m_vertexStream.primType(type);
}

void Renderer::solidColor(C3D_COLOR color)
{
    m_program.uniform4f("solidColor", color.r / 255.0f, color.g / 255.0f,
        color.b / 255.0f, color.a / 255.0f);
}

void Renderer::shadeMode(C3D_ESHADE mode)
{
    m_program.uniform1i("shadeMode", mode);
}

void Renderer::tmapEnable(C3D_BOOL enable)
{
    m_program.uniform1i("tmapEn", enable);
    if (enable) {
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
}

void Renderer::tmapSelect(C3D_HTX handle)
{
    m_tmap = handle;

    // unselect texture if handle is zero
    if (m_tmap == 0) {
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    // check if handle is correct
    TextureMap::iterator it = m_textures.find(m_tmap);
    if (it == m_textures.end()) {
        throw Error("Invalid texture handle", C3D_EC_BADPARAM);
    }

    // get texture object and bind it
    std::shared_ptr<Texture> texture = it->second;
    texture->bind();

    // send chroma key color to shader
    C3D_COLOR ck = texture->chromaKey();
    m_program.uniform3f(
        "chromaKey", ck.r / 255.0f, ck.g / 255.0f, ck.b / 255.0f);
}

void Renderer::tmapLight(C3D_ETLIGHT mode)
{
    m_program.uniform1i("tmapLight", mode);
}

void Renderer::tmapPerspCor(C3D_ETPERSPCOR mode)
{
    // TODO
}

void Renderer::tmapFilter(C3D_ETEXFILTER filter)
{
    m_sampler.parameteri(
        GL_TEXTURE_MAG_FILTER, GLCIF_TEXTURE_MAG_FILTER[filter]);
    m_sampler.parameteri(
        GL_TEXTURE_MIN_FILTER, GLCIF_TEXTURE_MIN_FILTER[filter]);
}

void Renderer::tmapTexOp(C3D_ETEXOP op)
{
    m_program.uniform1i("texOp", op);
}

void Renderer::alphaSrc(C3D_EASRC func)
{
    m_alphaSrc = func;
    glBlendFunc(GLCIF_BLEND_FUNC[m_alphaSrc], GLCIF_BLEND_FUNC[m_alphaDst]);
}

void Renderer::alphaDst(C3D_EADST func)
{
    m_alphaDst = func;
    glBlendFunc(GLCIF_BLEND_FUNC[m_alphaSrc], GLCIF_BLEND_FUNC[m_alphaDst]);
}

void Renderer::surfDrawPtr(C3D_PVOID ptr)
{
    // TODO
}

void Renderer::surfDrawPitch(C3D_UINT32 pitch)
{
    // TODO
}

void Renderer::surfDrawPixelFormat(C3D_EPIXFMT format)
{
    // TODO
}

void Renderer::surfVport(C3D_RECT vport)
{
    // TODO
}

void Renderer::fogEnable(C3D_BOOL enable)
{
    // TODO
}

void Renderer::ditherEnable(C3D_BOOL enable)
{
    // TODO
}

void Renderer::zCmpFunc(C3D_EZCMP func)
{
    if (func < C3D_EZCMP_MAX) {
        glDepthFunc(GLCIF_DEPTH_FUNC[func]);
    }
}

void Renderer::zMode(C3D_EZMODE mode)
{
    glDepthMask(GLCIF_DEPTH_MASK[mode]);

    if (mode > C3D_EZMODE_TESTON) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Renderer::surfZPtr(C3D_PVOID ptr)
{
    // TODO
}

void Renderer::surfZPitch(C3D_UINT32 pitch)
{
    // TODO
}

void Renderer::surfScissor(C3D_RECT scissor)
{
    // TODO
}

void Renderer::compositeEnable(C3D_BOOL enable)
{
    // TODO
}

void Renderer::compositeSelect(C3D_HTX select)
{
    // TODO
}

void Renderer::compositeFunc(C3D_ETEXCOMPFCN func)
{
    // TODO
}

void Renderer::compositeFactor(C3D_UINT32 fact)
{
    // TODO
}

void Renderer::compositeFilter(C3D_ETEXFILTER filter)
{
    // TODO
}

void Renderer::compositeFactorAlphaEnable(C3D_BOOL enable)
{
    // TODO
}

void Renderer::lodBiasLevel(C3D_UINT32 level)
{
    // TODO
}

void Renderer::alphaDstTestEnable(C3D_BOOL enable)
{
    // TODO
}

void Renderer::alphaDstTestFunc(C3D_EACMP func)
{
    // TODO
}

void Renderer::alphaDstWriteSelect(C3D_EASEL mode)
{
    // TODO
}

void Renderer::alphaDstReference(C3D_UINT32 ref)
{
    // TODO
}

void Renderer::specularEnable(C3D_BOOL enable)
{
    // TODO
}

void Renderer::enhancedColorRangeEnable(C3D_BOOL enable)
{
    // TODO
}

} // namespace cif
} // namespace glrage