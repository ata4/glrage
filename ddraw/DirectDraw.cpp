#include "DirectDraw.hpp"
#include "DirectDrawClipper.hpp"
#include "DirectDrawSurface.hpp"

#include "Logger.hpp"

namespace ddraw {

DirectDraw::DirectDraw()
{
    TRACE("DirectDraw::DirectDraw");
}

DirectDraw::~DirectDraw()
{
    TRACE("DirectDraw::~DirectDraw");
}

/*** IUnknown methods ***/
HRESULT WINAPI DirectDraw::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    TRACE("DirectDraw::QueryInterface");

    if (IsEqualGUID(riid, IID_IDirectDraw)) {
        *ppvObj = static_cast<IDirectDraw*>(this);
    } else if (IsEqualGUID(riid, IID_IDirectDraw2)) {
        *ppvObj = static_cast<IDirectDraw2*>(this);
    } else {
        return Unknown::QueryInterface(riid, ppvObj);
    }

    Unknown::AddRef();
    return DD_OK;
}

ULONG WINAPI DirectDraw::AddRef()
{
    return Unknown::AddRef();
}

ULONG WINAPI DirectDraw::Release()
{
    return Unknown::Release();
}

/*** IDirectDraw methods ***/
HRESULT WINAPI DirectDraw::Compact()
{
    TRACE("DirectDraw::Compact");

    return DD_OK;
}

HRESULT WINAPI DirectDraw::CreateClipper(
    DWORD dwFlags, LPDIRECTDRAWCLIPPER* lplpDDClipper, IUnknown* pUnkOuter)
{
    TRACE("DirectDraw::CreateClipper");

    *lplpDDClipper = new DirectDrawClipper();

    return DD_OK;
}

HRESULT WINAPI DirectDraw::CreatePalette(DWORD dwFlags,
    LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE* lplpDDPalette,
    IUnknown* pUnkOuter)
{
    TRACE("DirectDraw::CreatePalette");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc,
    LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter)
{
    TRACE("DirectDraw::CreateSurface");

    *lplpDDSurface = new DirectDrawSurface(*this, m_renderer, lpDDSurfaceDesc);

    return DD_OK;
}

HRESULT WINAPI DirectDraw::DuplicateSurface(
    LPDIRECTDRAWSURFACE lpDDSurface, LPDIRECTDRAWSURFACE* lplpDupDDSurface)
{
    TRACE("DirectDraw::DuplicateSurface");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::EnumDisplayModes(DWORD dwFlags,
    LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext,
    LPDDENUMMODESCALLBACK lpEnumModesCallback)
{
    TRACE("DirectDraw::EnumDisplayModes");

    if (lpDDSurfaceDesc) {
        // just give what the app wants
        lpEnumModesCallback(lpDDSurfaceDesc, lpContext);
    } else {
        DDSURFACEDESC desc;
        desc.dwSize = sizeof(DDSURFACEDESC);
        desc.dwWidth = DEFAULT_WIDTH;
        desc.dwHeight = DEFAULT_HEIGHT;
        desc.lPitch = desc.dwWidth * 2;

        lpEnumModesCallback(&desc, lpContext);
    }

    return DD_OK;
}

HRESULT WINAPI DirectDraw::EnumSurfaces(DWORD dwFlags,
    LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext,
    LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback)
{
    TRACE("DirectDraw::EnumSurfaces");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::FlipToGDISurface()
{
    TRACE("DirectDraw::FlipToGDISurface");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::GetCaps(
    LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps)
{
    TRACE("DirectDraw::GetCaps");

    LPDDCAPS ps[2] = {lpDDDriverCaps, lpDDHELCaps};
    for (uint32_t i = 0; i < 2; i++) {
        LPDDCAPS p = ps[i];
        if (!p) {
            continue;
        }

        p->dwZBufferBitDepths = DDBD_16 | DDBD_32;
        p->dwVidMemTotal = p->dwVidMemFree = 4 << 20;
        p->ddsCaps.dwCaps =
            DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER;
    }

    return DD_OK;
}

HRESULT WINAPI DirectDraw::GetDisplayMode(LPDDSURFACEDESC lpDDSurfaceDesc)
{
    TRACE("DirectDraw::GetDisplayMode");

    LPDDSURFACEDESC desc = lpDDSurfaceDesc;

    desc->ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc->ddpfPixelFormat.dwRGBBitCount = 16;
    desc->ddpfPixelFormat.dwRBitMask = 15 << 10;
    desc->ddpfPixelFormat.dwGBitMask = 15 << 5;
    desc->ddpfPixelFormat.dwBBitMask = 15;
    desc->ddpfPixelFormat.dwRGBAlphaBitMask = 1 << 15;

    desc->dwWidth = m_width;
    desc->dwHeight = m_height;
    desc->lPitch = m_width * (desc->ddpfPixelFormat.dwRGBBitCount / 8);

    desc->dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH | DDSD_PIXELFORMAT;

    return DD_OK;
}

HRESULT WINAPI DirectDraw::GetFourCCCodes(LPDWORD lpNumCodes, LPDWORD lpCodes)
{
    TRACE("DirectDraw::GetFourCCCodes");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::GetGDISurface(LPDIRECTDRAWSURFACE* lplpGDIDDSSurface)
{
    TRACE("DirectDraw::GetGDISurface");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::GetMonitorFrequency(LPDWORD lpdwFrequency)
{
    TRACE("DirectDraw::GetMonitorFrequency");

    if (lpdwFrequency) {
        *lpdwFrequency = m_refreshRate;
    }

    return DD_OK;
}

HRESULT WINAPI DirectDraw::GetScanLine(LPDWORD lpdwScanLine)
{
    TRACE("DirectDraw::GetScanLine");

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDraw::GetVerticalBlankStatus(LPBOOL lpbIsInVB)
{
    TRACE("DirectDraw::GetVerticalBlankStatus");

    if (lpbIsInVB) {
        *lpbIsInVB = TRUE;
    }

    return DD_OK;
}

HRESULT WINAPI DirectDraw::Initialize(GUID* lpGUID)
{
    TRACE("DirectDraw::Initialize");

    return DD_OK;
}

HRESULT WINAPI DirectDraw::RestoreDisplayMode()
{
    TRACE("DirectDraw::RestoreDisplayMode");

    // nothing to do, desktop display is never touched
    return DD_OK;
}

HRESULT WINAPI DirectDraw::SetCooperativeLevel(HWND hWnd, DWORD dwFlags)
{
    TRACE("DirectDraw::SetCooperativeLevel");

    m_context.setFullscreen(dwFlags & DDSCL_FULLSCREEN);

    return DD_OK;
}

HRESULT WINAPI DirectDraw::SetDisplayMode(
    DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
    TRACE("DirectDraw::SetDisplayMode");

    return SetDisplayMode(dwWidth, dwHeight, dwBPP, DEFAULT_REFRESH_RATE, 0);
}

HRESULT WINAPI DirectDraw::WaitForVerticalBlank(DWORD dwFlags, HANDLE hEvent)
{
    TRACE("DirectDraw::WaitForVerticalBlank");

    return DD_OK;
}

/*** IDirectDraw2 methods ***/
HRESULT WINAPI DirectDraw::SetDisplayMode(DWORD dwWidth, DWORD dwHeight,
    DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags)
{
    TRACE("DirectDraw2::SetDisplayMode");

    m_width = dwWidth;
    m_height = dwHeight;
    m_bits = dwBPP;
    m_refreshRate = dwRefreshRate;

    m_context.setDisplaySize(m_width, m_height);

    return DD_OK;
}

HRESULT WINAPI DirectDraw::GetAvailableVidMem(
    LPDDSCAPS lpDDSCaps, LPDWORD lpdwTotal, LPDWORD lpdwFree)
{
    TRACE("DirectDraw2::GetAvailableVidMem");

    // just return 8 MiB, which is plenty for mid-90s hardware
    *lpdwTotal = *lpdwFree = 8 << 20;

    return DD_OK;
}

} // namespace ddraw