#include "ati3dcif.h"
#include "Error.hpp"
#include "Renderer.hpp"
#include "Utils.hpp"

#include <glrage\GLRage.hpp>
#include <glrage_util\ErrorUtils.hpp>
#include <glrage_util\Logger.hpp>

#include <stdexcept>

namespace glrage {
namespace cif {

static Context& context = GLRageGetContext();
static Renderer* renderer = nullptr;
static bool contextCreated = false;

C3D_EC
HandleException()
{
    try {
        throw;
    } catch (const Error& ex) {
#ifdef _DEBUG
        ErrorUtils::warning(ex);
#else
        LOG_INFO("CIF error: %s (0x%x %s)", ex.what(), ex.getErrorCode(),
            ex.getErrorName());
#endif
        return ex.getErrorCode();
    } catch (const std::runtime_error& ex) {
        ErrorUtils::warning(ex);
        return C3D_EC_GENFAIL;
    } catch (const std::logic_error& ex) {
        ErrorUtils::warning(ex);
        return C3D_EC_GENFAIL;
    }
}

extern "C" {

EXPORT(ATI3DCIF_Init, C3D_EC, (void))
{
    LOG_TRACE("");

    context.init();
    context.attach();

    ErrorUtils::setHWnd(context.getHWnd());

    // do some cleanup in case the app forgets to call ATI3DCIF_Term
    if (renderer) {
        LOG_INFO("Previous instance was not terminated by ATI3DCIF_Term!");
        ATI3DCIF_Term();
    }

    try {
        renderer = new Renderer();
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_Term, C3D_EC, (void))
{
    LOG_TRACE("");

    try {
        if (renderer) {
            delete renderer;
            renderer = nullptr;
        }
    } catch (...) {
        return HandleException();
    }

    // SDK PDF says "TRUE if successful, otherwise FALSE", but the
    // function uses C3D_EC as return value.
    // In other words: TRUE = C3D_EC_GENFAIL and FALSE = C3D_EC_OK? WTF...
    // Anyway, most apps don't seem to care about the return value
    // of this function, so stick with C3D_EC_OK for now.
    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_GetInfo, C3D_EC, (PC3D_3DCIFINFO p3DCIFInfo))
{
    LOG_TRACE("");

    // check for invalid struct
    if (!p3DCIFInfo || p3DCIFInfo->u32Size > 48) {
        return C3D_EC_BADPARAM;
    }

    // values from an ATI Xpert 98 with a 3D Rage Pro AGP 2x
    p3DCIFInfo->u32FrameBuffBase =
        0; // Host pointer to frame buffer base (TODO: allocate memory?)
    p3DCIFInfo->u32OffScreenHeap =
        0; // Host pointer to offscreen heap (TODO: allocate memory?)
    p3DCIFInfo->u32OffScreenSize = 0x4fe800; // Size of offscreen heap
    p3DCIFInfo->u32TotalRAM = 8 << 20;       // Total amount of RAM on the card
    p3DCIFInfo->u32ASICID = 0x409;           // ASIC Id. code
    p3DCIFInfo->u32ASICRevision = 0x47ff;    // ASIC revision

    // older CIF versions don't have CIF caps, so check the size first
    if (p3DCIFInfo->u32Size == 48) {
        // note: 0x400 and 0x800 are reported in 4.10.2690 and later but are not
        // defined in ATI3DCIF.H
        p3DCIFInfo->u32CIFCaps1 = C3D_CAPS1_FOG | C3D_CAPS1_POINT |
                                  C3D_CAPS1_RECT | C3D_CAPS1_Z_BUFFER |
                                  C3D_CAPS1_CI4_TMAP | C3D_CAPS1_CI8_TMAP |
                                  C3D_CAPS1_DITHER_EN | C3D_CAPS1_ENH_PERSP |
                                  C3D_CAPS1_SCISSOR | 0x400 | 0x800;
        p3DCIFInfo->u32CIFCaps2 = C3D_CAPS2_TEXTURE_CLAMP |
                                  C3D_CAPS2_DESTINATION_ALPHA_BLEND |
                                  C3D_CAPS2_TEXTURE_TILING;

        // unused caps
        p3DCIFInfo->u32CIFCaps3 = 0;
        p3DCIFInfo->u32CIFCaps4 = 0;
        p3DCIFInfo->u32CIFCaps5 = 0;
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_TextureReg, C3D_EC, (C3D_PTMAP ptmapToReg, C3D_PHTX phtmap))
{
    LOG_TRACE("0x%p, 0x%p", *ptmapToReg, *phtmap);

    try {
        renderer->textureReg(ptmapToReg, phtmap);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_TextureUnreg, C3D_EC, (C3D_HTX htxToUnreg))
{
    LOG_TRACE("0x%p", htxToUnreg);

    try {
        renderer->textureUnreg(htxToUnreg);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_TexturePaletteCreate, C3D_EC,
    (C3D_ECI_TMAP_TYPE epalette, void* pPalette, C3D_PHTXPAL phtpalCreated))
{
    LOG_TRACE("%s, 0x%p, 0x%p", cif::C3D_ECI_TMAP_TYPE_NAMES[epalette],
        pPalette, phtpalCreated);

    try {
        renderer->texturePaletteCreate(epalette, pPalette, phtpalCreated);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_TexturePaletteDestroy, C3D_EC, (C3D_HTXPAL htxpalToDestroy))
{
    LOG_TRACE("0x%p", htxpalToDestroy);

    try {
        renderer->texturePaletteDestroy(htxpalToDestroy);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_TexturePaletteAnimate, C3D_EC,
    (C3D_HTXPAL htxpalToAnimate, C3D_UINT32 u32StartIndex,
        C3D_UINT32 u32NumEntries, C3D_PPALETTENTRY pclrPalette))
{
    LOG_TRACE("0x%p, %d, %d, 0x%p", htxpalToAnimate, u32StartIndex,
        u32NumEntries, *pclrPalette);

    try {
        renderer->texturePaletteAnimate(
            htxpalToAnimate, u32StartIndex, u32NumEntries, pclrPalette);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_ContextCreate, C3D_HRC, (void))
{
    LOG_TRACE("");

    context.attach();

    // can't create more than one context
    if (contextCreated) {
        return nullptr;
    }

    try {
        // new context, set default states
        C3D_COLOR black = {0};
        C3D_RECT rect = {0};

        renderer->fogColor(black);
        renderer->vertexType(C3D_EV_VTCF);
        renderer->primType(C3D_EPRIM_TRI);
        renderer->solidColor(black);
        renderer->shadeMode(C3D_ESH_SMOOTH);
        renderer->tmapEnable(FALSE);
        renderer->tmapSelect(nullptr);
        renderer->tmapLight(C3D_ETL_NONE);
        renderer->tmapPerspCor(C3D_ETPC_THREE);
        renderer->tmapFilter(C3D_ETFILT_MINPNT_MAG2BY2);
        renderer->tmapTexOp(C3D_ETEXOP_NONE);
        renderer->alphaSrc(C3D_EASRC_ONE);
        renderer->alphaDst(C3D_EADST_ZERO);
        renderer->surfDrawPtr(nullptr);
        renderer->surfDrawPitch(0);
        renderer->surfDrawPixelFormat(C3D_EPF_RGB8888);
        renderer->surfVport(rect);
        renderer->fogEnable(FALSE);
        renderer->ditherEnable(TRUE);
        renderer->zCmpFunc(C3D_EZCMP_ALWAYS);
        renderer->zMode(C3D_EZMODE_OFF);
        renderer->surfZPtr(nullptr);
        renderer->surfZPitch(0);
        renderer->surfScissor(rect);
        renderer->compositeEnable(FALSE);
        renderer->compositeSelect(nullptr);
        renderer->compositeFunc(C3D_ETEXCOMPFCN_MAX);
        renderer->compositeFactor(8);
        renderer->compositeFilter(C3D_ETFILT_MIN2BY2_MAG2BY2);
        renderer->compositeFactorAlphaEnable(FALSE);
        renderer->lodBiasLevel(0);
        renderer->alphaDstTestEnable(FALSE);
        renderer->alphaDstTestFunc(C3D_EACMP_ALWAYS);
        renderer->alphaDstWriteSelect(C3D_EASEL_ZERO);
        renderer->alphaDstReference(0);
        renderer->specularEnable(FALSE);
        renderer->enhancedColorRangeEnable(FALSE);
    } catch (...) {
        HandleException();
        return nullptr;
    }

    contextCreated = true;

    // According to ATI3DCIF.H, "only one context may be exist at a time",
    // so always returning 1 should be fine
    return (C3D_HRC)1;
}

EXPORT(ATI3DCIF_ContextDestroy, C3D_EC, (C3D_HRC hRC))
{
    LOG_TRACE("0x%p", hRC);

    // can't destroy a context that wasn't created
    if (!contextCreated) {
        return C3D_EC_BADPARAM;
    }

    contextCreated = false;

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_ContextSetState, C3D_EC,
    (C3D_HRC hRC, C3D_ERSID eRStateID, C3D_PRSDATA pRStateData))
{
#ifdef DEBUG_TRACE
    std::string stateDataStr =
        cif::Utils::dumpRenderStateData(eRStateID, pRStateData);
    LOG_TRACE("0x%p, %s, %s", hRC, cif::C3D_ERSID_NAMES[eRStateID],
        stateDataStr.c_str());
#endif

    try {
        switch (eRStateID) {
            case C3D_ERS_FG_CLR:
                renderer->fogColor(*static_cast<C3D_PCOLOR>(pRStateData));
                break;
            case C3D_ERS_VERTEX_TYPE:
                renderer->vertexType(*static_cast<C3D_PEVERTEX>(pRStateData));
                break;
            case C3D_ERS_PRIM_TYPE:
                renderer->primType(*static_cast<C3D_PEPRIM>(pRStateData));
                break;
            case C3D_ERS_SOLID_CLR:
                renderer->solidColor(*static_cast<C3D_PCOLOR>(pRStateData));
                break;
            case C3D_ERS_SHADE_MODE:
                renderer->shadeMode(*static_cast<C3D_PESHADE>(pRStateData));
                break;
            case C3D_ERS_TMAP_EN:
                renderer->tmapEnable(*static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_TMAP_SELECT:
                renderer->tmapSelect(*static_cast<C3D_PHTX>(pRStateData));
                break;
            case C3D_ERS_TMAP_LIGHT:
                renderer->tmapLight(*static_cast<C3D_PETLIGHT>(pRStateData));
                break;
            case C3D_ERS_TMAP_PERSP_COR:
                renderer->tmapPerspCor(
                    *static_cast<C3D_PETPERSPCOR>(pRStateData));
                break;
            case C3D_ERS_TMAP_FILTER:
                renderer->tmapFilter(
                    *static_cast<C3D_PETEXFILTER>(pRStateData));
                break;
            case C3D_ERS_TMAP_TEXOP:
                renderer->tmapTexOp(*static_cast<C3D_PETEXOP>(pRStateData));
                break;
            case C3D_ERS_ALPHA_SRC:
                renderer->alphaSrc(*static_cast<C3D_PEASRC>(pRStateData));
                break;
            case C3D_ERS_ALPHA_DST:
                renderer->alphaDst(*static_cast<C3D_PEADST>(pRStateData));
                break;
            case C3D_ERS_SURF_DRAW_PTR:
                renderer->surfDrawPtr(static_cast<C3D_PVOID>(pRStateData));
                break;
            case C3D_ERS_SURF_DRAW_PITCH:
                renderer->surfDrawPitch(*static_cast<C3D_PUINT32>(pRStateData));
                break;
            case C3D_ERS_SURF_DRAW_PF:
                renderer->surfDrawPixelFormat(
                    *static_cast<C3D_PEPIXFMT>(pRStateData));
                break;
            case C3D_ERS_SURF_VPORT:
                renderer->surfVport(*static_cast<C3D_PRECT>(pRStateData));
                break;
            case C3D_ERS_FOG_EN:
                renderer->fogEnable(*static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_DITHER_EN:
                renderer->ditherEnable(*static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_Z_CMP_FNC:
                renderer->zCmpFunc(*static_cast<C3D_PEZCMP>(pRStateData));
                break;
            case C3D_ERS_Z_MODE:
                renderer->zMode(*static_cast<C3D_PEZMODE>(pRStateData));
                break;
            case C3D_ERS_SURF_Z_PTR:
                renderer->surfZPtr(static_cast<C3D_PVOID>(pRStateData));
                break;
            case C3D_ERS_SURF_Z_PITCH:
                renderer->surfZPitch(*static_cast<C3D_PUINT32>(pRStateData));
                break;
            case C3D_ERS_SURF_SCISSOR:
                renderer->surfScissor(*static_cast<C3D_PRECT>(pRStateData));
                break;
            case C3D_ERS_COMPOSITE_EN:
                renderer->compositeEnable(*static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_COMPOSITE_SELECT:
                renderer->compositeSelect(*static_cast<C3D_PHTX>(pRStateData));
                break;
            case C3D_ERS_COMPOSITE_FNC:
                renderer->compositeFunc(
                    *static_cast<C3D_PETEXCOMPFCN>(pRStateData));
                break;
            case C3D_ERS_COMPOSITE_FACTOR:
                renderer->compositeFactor(
                    *static_cast<C3D_PUINT32>(pRStateData));
                break;
            case C3D_ERS_COMPOSITE_FILTER:
                renderer->compositeFilter(
                    *static_cast<C3D_PETEXFILTER>(pRStateData));
                break;
            case C3D_ERS_COMPOSITE_FACTOR_ALPHA:
                renderer->compositeFactorAlphaEnable(
                    *static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_LOD_BIAS_LEVEL:
                renderer->lodBiasLevel(*static_cast<C3D_PUINT32>(pRStateData));
                break;
            case C3D_ERS_ALPHA_DST_TEST_ENABLE:
                renderer->alphaDstTestEnable(
                    *static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_ALPHA_DST_TEST_FNC:
                renderer->alphaDstTestFunc(
                    *static_cast<C3D_PEACMP>(pRStateData));
                break;
            case C3D_ERS_ALPHA_DST_WRITE_SELECT:
                renderer->alphaDstWriteSelect(
                    *static_cast<C3D_PEASEL>(pRStateData));
                break;
            case C3D_ERS_ALPHA_DST_REFERENCE:
                renderer->alphaDstReference(
                    *static_cast<C3D_PUINT32>(pRStateData));
                break;
            case C3D_ERS_SPECULAR_EN:
                renderer->specularEnable(*static_cast<C3D_PBOOL>(pRStateData));
                break;
            case C3D_ERS_ENHANCED_COLOR_RANGE_EN:
                renderer->enhancedColorRangeEnable(
                    *static_cast<C3D_PBOOL>(pRStateData));
                break;
        }
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_RenderBegin, C3D_EC, (C3D_HRC hRC))
{
    LOG_TRACE("0x%p", hRC);

    context.renderBegin();

    try {
        renderer->renderBegin(hRC);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_RenderEnd, C3D_EC, (void))
{
    LOG_TRACE("");

    try {
        renderer->renderEnd();
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_RenderSwitch, C3D_EC, (C3D_HRC hRC))
{
    LOG_TRACE("0x%p", hRC);
    // function has officially never been implemented
    return C3D_EC_NOTIMPYET;
}

EXPORT(ATI3DCIF_RenderPrimStrip, C3D_EC,
    (C3D_VSTRIP vStrip, C3D_UINT32 u32NumVert))
{
    LOG_TRACE("0x%p, %d", vStrip, u32NumVert);

    try {
        renderer->renderPrimStrip(vStrip, u32NumVert);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(
    ATI3DCIF_RenderPrimList, C3D_EC, (C3D_VLIST vList, C3D_UINT32 u32NumVert))
{
    LOG_TRACE("0x%p, %d", vList, u32NumVert);

    try {
        renderer->renderPrimList(vList, u32NumVert);
    } catch (...) {
        return HandleException();
    }

    return C3D_EC_OK;
}

EXPORT(ATI3DCIF_RenderPrimMesh, C3D_EC,
    (C3D_PVARRAY vMesh, C3D_PUINT32 pu32Indicies, C3D_UINT32 u32NumIndicies))
{
    LOG_TRACE("0x%p, %d", vMesh, u32NumIndicies);

    return C3D_EC_OK;
}

} // extern "C"

} // namespace cif
} // namespace glrage