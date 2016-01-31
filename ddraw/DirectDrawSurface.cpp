#include "DirectDrawSurface.hpp"
#include "DirectDrawClipper.hpp"

#include "Logger.hpp"

#include <algorithm>

namespace ddraw {

DirectDrawSurface::DirectDrawSurface(DirectDraw& lpDD, SurfaceRenderer& renderer, LPDDSURFACEDESC lpDDSurfaceDesc) :
    m_dd(lpDD),
    m_renderer(renderer),
    m_desc(*lpDDSurfaceDesc)
{
    TRACE("DirectDrawSurface::DirectDrawSurface");

    m_dd.AddRef();

    DDSURFACEDESC displayDesc;
    m_dd.GetDisplayMode(&displayDesc);

    // use display size if surface has no defined dimensions
    if (!(m_desc.dwFlags & (DDSD_WIDTH | DDSD_HEIGHT))) {
        m_desc.dwWidth = displayDesc.dwWidth;
        m_desc.dwHeight = displayDesc.dwHeight;
        m_desc.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT;
    }

    // use display pixel format if surface has no defined pixel format
    if (!(m_desc.dwFlags & DDSD_PIXELFORMAT)) {
        m_desc.ddpfPixelFormat = displayDesc.ddpfPixelFormat;
        m_desc.dwFlags |= DDSD_PIXELFORMAT;
    }

    // calculate pitch if surface has no defined pitch
    if (!(m_desc.dwFlags & DDSD_PITCH)) {
        m_desc.lPitch = m_desc.dwWidth * (m_desc.ddpfPixelFormat.dwRGBBitCount / 8);
        m_desc.dwFlags |= DDSD_PITCH;
    }

    // allocate surface buffer
    m_buffer.resize(m_desc.lPitch * m_desc.dwHeight, 0);
    m_desc.lpSurface = nullptr;

    // attach back buffer if defined
    if (m_desc.dwFlags & DDSD_BACKBUFFERCOUNT && m_desc.dwBackBufferCount > 0) {
        LOG("DirectDrawSurface::DirectDrawSurface: found DDSD_BACKBUFFERCOUNT, creating back buffer");

        DDSURFACEDESC backBufferDesc = m_desc;
        backBufferDesc.ddsCaps.dwCaps |= DDSCAPS_BACKBUFFER | DDSCAPS_FLIP;
        backBufferDesc.ddsCaps.dwCaps &= ~(DDSCAPS_FRONTBUFFER | DDSCAPS_VISIBLE);
        backBufferDesc.dwFlags &= ~DDSD_BACKBUFFERCOUNT;
        backBufferDesc.dwBackBufferCount = 0;
        m_backBuffer = new DirectDrawSurface(lpDD, m_renderer, &backBufferDesc);

        m_desc.ddsCaps.dwCaps |= DDSCAPS_FRONTBUFFER | DDSCAPS_FLIP | DDSCAPS_VISIBLE;
    }
}

DirectDrawSurface::~DirectDrawSurface() {
    TRACE("DirectDrawSurface::~DirectDrawSurface");

    if (m_backBuffer) {
        m_backBuffer->Release();
        m_backBuffer = nullptr;
    }

    if (m_depthBuffer) {
        m_depthBuffer->Release();
        m_depthBuffer = nullptr;
    }

    m_dd.Release();

    if (m_desc.lpSurface) {
        m_desc.lpSurface = nullptr;
    }
}

/*** IUnknown methods ***/
HRESULT WINAPI DirectDrawSurface::QueryInterface(REFIID riid, LPVOID* ppvObj) {
    TRACE("DirectDrawSurface::QueryInterface");
    if (IsEqualGUID(riid, IID_IDirectDrawSurface)) {
        *ppvObj = static_cast<IDirectDrawSurface*>(this);
    } else if (IsEqualGUID(riid, IID_IDirectDrawSurface2)) {
        *ppvObj = static_cast<IDirectDrawSurface2*>(this);
    } else {
        return Unknown::QueryInterface(riid, ppvObj);
    }

    Unknown::AddRef();
    return S_OK;
}

ULONG WINAPI DirectDrawSurface::AddRef() {
    return Unknown::AddRef();
}

ULONG WINAPI DirectDrawSurface::Release() {
    return Unknown::Release();
}


/*** IDirectDrawSurface methods ***/
HRESULT WINAPI DirectDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE lpDDSAttachedSurface) {
    TRACE("DirectDrawSurface::AddAttachedSurface");

    if (!lpDDSAttachedSurface) {
        return DDERR_INVALIDOBJECT;
    }

    DirectDrawSurface* ps = static_cast<DirectDrawSurface*>(lpDDSAttachedSurface);
    DWORD caps = ps->m_desc.ddsCaps.dwCaps;
    if (caps & DDSCAPS_ZBUFFER) {
        m_depthBuffer = ps;
    } else if (caps & DDSCAPS_BACKBUFFER) {
        m_backBuffer = ps;
    } else {
        return DDERR_CANNOTATTACHSURFACE;
    }

    ps->AddRef();
    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::AddOverlayDirtyRect(LPRECT lpRect) {
    TRACE("DirectDrawSurface::AddOverlayDirtyRect");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) {
    TRACE("DirectDrawSurface::Blt");

    // can't blit while locked
    if (m_locked) {
        return DDERR_LOCKEDSURFACES;
    }

    if (lpDDSrcSurface) {
        // first check if the blit will replace the entire buffer
        bool replace = true;

        if (lpSrcRect && (lpSrcRect->left != 0 || lpSrcRect->right != m_desc.dwWidth || lpSrcRect->top != 0 || lpSrcRect->bottom != m_desc.dwHeight)) {
            replace = false;
        }

        if (lpDestRect && (lpDestRect->left != 0 || lpDestRect->right != m_desc.dwWidth || lpDestRect->top != 0 || lpDestRect->bottom != m_desc.dwHeight)) {
            replace = false;
        }

        if (replace) {
            DirectDrawSurface* src = static_cast<DirectDrawSurface*>(lpDDSrcSurface);
            m_buffer = src->m_buffer;
        } else {
            // TODO: rectangular 2D copy
        }

        m_dirty = true;
    }

    // Clear primary surface in 2D mode ony. OpenGL already does the clearing
    // on hardware in 3D, so it would be a waste of CPU time.
    if (m_context.isRendered() && m_desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) {
        return DD_OK;
    }

    if (dwFlags & DDBLT_COLORFILL) {
        clear(lpDDBltFx->dwFillColor);
    }

    if (dwFlags & DDBLT_DEPTHFILL && m_depthBuffer) {
        m_depthBuffer->clear(0);
    }

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::BltBatch(LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags) {
    TRACE("DirectDrawSurface::BltBatch");

    // can't blit while locked
    if (m_locked) {
        return DDERR_LOCKEDSURFACES;
    }

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) {
    TRACE("DirectDrawSurface::BltFast");

    // can't blit while locked
    if (m_locked) {
        return DDERR_LOCKEDSURFACES;
    }

    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSurface) {
    TRACE("DirectDrawSurface::DeleteAttachedSurface");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback) {
    TRACE("DirectDrawSurface::EnumAttachedSurfaces");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK lpfnCallback) {
    TRACE("DirectDrawSurface::EnumOverlayZOrders");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::Flip(LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride, DWORD dwFlags) {
    TRACE("DirectDrawSurface::Flip");

    // check if the surface can be flipped
    if (!(m_desc.ddsCaps.dwCaps & DDSCAPS_FLIP) || !(m_desc.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER) || !m_backBuffer) {
        return DDERR_NOTFLIPPABLE;
    }

    bool rendered = m_context.isRendered();

    // don't re-upload surfaces if external rendering was active after lock() has
    // been called, since it wouldn't be visible anyway
    if (rendered) {
        m_dirty = false;
    }

    // swap front and back buffers
    // TODO: use buffer chain correctly
    // TODO: use lpDDSurfaceTargetOverride when defined
    m_buffer.swap(m_backBuffer->m_buffer);

    bool dirtyTmp = m_dirty;
    m_dirty = m_backBuffer->m_dirty;
    m_backBuffer->m_dirty = dirtyTmp;

    // upload surface if dirty
    if (m_dirty) {
        m_renderer.upload(m_desc, m_buffer);
        m_dirty = false;
    }
    
    // swap buffer now if there was external rendering, otherwise the surface
    // would overwrite it
    if (rendered) {
        m_context.swapBuffers();
    }

    // update viewport in case the window size has changed
    m_context.setupViewport();

    // render surface
    m_renderer.render();

    // swap buffer after the surface has been rendered if there was no external
    // rendering for this frame, fixes title screens and other pure 2D operations
    // that aren't continuously updated
    if (!rendered) {
        m_context.swapBuffers();
    }

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE* lplpDDAttachedSurface) {
    TRACE("DirectDrawSurface::GetAttachedSurface");

    if (lpDDSCaps->dwCaps & DDSCAPS_BACKBUFFER) {
        *lplpDDAttachedSurface = m_backBuffer;
        return DD_OK;
    }

    if (lpDDSCaps->dwCaps & DDSCAPS_ZBUFFER) {
        *lplpDDAttachedSurface = m_depthBuffer;
        return DD_OK;
    }

    return DDERR_SURFACENOTATTACHED;
}

HRESULT WINAPI DirectDrawSurface::GetBltStatus(DWORD dwFlags) {
    TRACE("DirectDrawSurface::GetBltStatus");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetCaps(LPDDSCAPS lpDDSCaps) {
    TRACE("DirectDrawSurface::GetCaps");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetClipper(LPDIRECTDRAWCLIPPER* lplpDDClipper) {
    TRACE("DirectDrawSurface::GetClipper");

    *lplpDDClipper = reinterpret_cast<LPDIRECTDRAWCLIPPER>(m_clipper);

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
    TRACE("DirectDrawSurface::GetColorKey");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetDC(HDC* phDC) {
    TRACE("DirectDrawSurface::GetDC");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetFlipStatus(DWORD dwFlags) {
    TRACE("DirectDrawSurface::GetFlipStatus");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetOverlayPosition(LPLONG lplX, LPLONG lplY) {
    TRACE("DirectDrawSurface::GetOverlayPosition");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetPalette(LPDIRECTDRAWPALETTE* lplpDDPalette) {
    TRACE("DirectDrawSurface::GetPalette");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat) {
    TRACE("DirectDrawSurface::GetPixelFormat");

    *lpDDPixelFormat = m_desc.ddpfPixelFormat;

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc) {
    TRACE("DirectDrawSurface::GetSurfaceDesc");

    *lpDDSurfaceDesc = m_desc;

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC lpDDSurfaceDesc) {
    TRACE("DirectDrawSurface::Initialize");

    // "This method is provided for compliance with the Component Object Model (COM).
    // Because the DirectDrawSurface object is initialized when it is created, this method always
    // returns DDERR_ALREADYINITIALIZED."
    return DDERR_ALREADYINITIALIZED;
}

HRESULT WINAPI DirectDrawSurface::IsLost() {
    TRACE("DirectDrawSurface::IsLost");

    // we're never lost..
    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) {
    TRACEF("DirectDrawSurface::Lock(%p, %p, %d, %p)", lpDestRect, lpDDSurfaceDesc, dwFlags, hEvent);

    // ensure that the surface is not already locked
    if (m_locked) {
        return DDERR_SURFACEBUSY;
    }

    // assign lpSurface
    m_desc.lpSurface = &m_buffer[0];
    m_desc.dwFlags |= DDSD_LPSURFACE;

    m_locked = true;
    m_dirty = true;

    *lpDDSurfaceDesc = m_desc;

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::ReleaseDC(HDC hDC) {
    TRACE("DirectDrawSurface::ReleaseDC");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::Restore() {
    TRACE("DirectDrawSurface::Restore");

    // we can't lose surfaces..
    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper) {
    TRACE("DirectDrawSurface::SetClipper");

    m_clipper = reinterpret_cast<DirectDrawClipper*>(lpDDClipper);

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey) {
    TRACE("DirectDrawSurface::SetColorKey");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::SetOverlayPosition(LONG lX, LONG lY) {
    TRACE("DirectDrawSurface::SetOverlayPosition");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette) {
    TRACE("DirectDrawSurface::SetPalette");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::Unlock(LPVOID lp) {
    TRACE("DirectDrawSurface::Unlock");

    // ensure that the surface is actually locked
    if (!m_locked) {
        return DDERR_NOTLOCKED;
    }

    // unassign lpSurface
    m_desc.lpSurface = nullptr;
    m_desc.dwFlags &= ~DDSD_LPSURFACE;

    m_locked = false;

    // re-draw stand-alone back buffers immediately after unlocking (used for video sequences)
    if (m_desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE && !(m_desc.ddsCaps.dwCaps & DDSCAPS_FLIP)) {
        // duplicate even to odd lines
        //for (uint32_t i = 0; i < m_desc.dwHeight; i += 2) {
        //    memcpy(m_buffer + (i + 1) * m_desc.lPitch, m_buffer + i * m_desc.lPitch, m_desc.lPitch);
        //}
        m_context.swapBuffers();
        m_context.setupViewport();
        m_renderer.upload(m_desc, m_buffer);
        m_renderer.render();
    }

    return DD_OK;
}

HRESULT WINAPI DirectDrawSurface::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx) {
    TRACE("DirectDrawSurface::UpdateOverlay");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::UpdateOverlayDisplay(DWORD dwFlags) {
    TRACE("DirectDrawSurface::UpdateOverlayDisplay");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE lpDDSReference) {
    TRACE("DirectDrawSurface::UpdateOverlayZOrder");
    return DDERR_UNSUPPORTED;
}

/*** IDirectDrawSurface2 methods ***/
HRESULT WINAPI DirectDrawSurface::AddAttachedSurface(LPDIRECTDRAWSURFACE2 lpDDSAttachedSurface) {
    TRACE("DirectDrawSurface2::AddAttachedSurface");
    return DDERR_UNSUPPORTED;
}
 
HRESULT WINAPI DirectDrawSurface::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE2 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) {
    TRACE("DirectDrawSurface2::Blt");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE2 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) {
    TRACE("DirectDrawSurface2::BltFast");
    return DDERR_UNSUPPORTED;
}
 
HRESULT WINAPI DirectDrawSurface::DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE2 lpDDSurface) {
    TRACE("DirectDrawSurface2::DeleteAttachedSurface");
    return DDERR_UNSUPPORTED;
}


HRESULT WINAPI DirectDrawSurface::Flip(LPDIRECTDRAWSURFACE2 lpDDSurfaceTargetOverride, DWORD dwFlags) {
    TRACE("DirectDrawSurface2::Flip");
    return DDERR_UNSUPPORTED;
}
 
HRESULT WINAPI DirectDrawSurface::GetAttachedSurface(LPDDSCAPS lpDDSCaps, LPDIRECTDRAWSURFACE2* lplpDDAttachedSurface) {
    TRACE("DirectDrawSurface2::GetAttachedSurface");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE2 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx) {
    TRACE("DirectDrawSurface2::UpdateOverlay");
    return DDERR_UNSUPPORTED;
}

HRESULT WINAPI DirectDrawSurface::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE2 lpDDSReference) {
    TRACE("DirectDrawSurface2::UpdateOverlayZOrder");
    return DDERR_UNSUPPORTED;
}
 
HRESULT WINAPI DirectDrawSurface::GetDDInterface(LPVOID* lplpDD) {
    TRACE("DirectDrawSurface2::GetDDInterface");
    return DDERR_UNSUPPORTED;
}
 
HRESULT WINAPI DirectDrawSurface::PageLock(DWORD dwFlags) {
    TRACE("DirectDrawSurface2::PageLock");
    return DDERR_UNSUPPORTED;
}
 
HRESULT WINAPI DirectDrawSurface::PageUnlock(DWORD dwFlags) {
    TRACE("DirectDrawSurface2::PageUnlock");
    return DDERR_UNSUPPORTED;
}

/*** Custom methods ***/
void DirectDrawSurface::clear(int32_t color) {
    if (m_desc.ddpfPixelFormat.dwRGBBitCount == 8 || color == 0) {
        // clear() may be called frequently on potentially large buffers, so use
        // memset instead of std::fill
        memset(&m_buffer[0], color & 0xff, m_buffer.size());
    } else if (m_desc.ddpfPixelFormat.dwRGBBitCount % 8 == 0) {
        int32_t i = 0;
        std::generate(m_buffer.begin(), m_buffer.end(), [this, &i, &color]() {
            int32_t colorOffset = i++ * 8 % this->m_desc.ddpfPixelFormat.dwRGBBitCount;
            return (color >> colorOffset) & 0xff;
        });
    } else {
        // TODO: support odd bit counts?
    }

    m_dirty = true;
}

}