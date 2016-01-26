#include "DirectDrawClipper.hpp"

#include "Logger.hpp"

namespace ddraw {

DirectDrawClipper::DirectDrawClipper() {
    TRACE("DirectDrawClipper::DirectDrawClipper");
}

DirectDrawClipper::~DirectDrawClipper() {
    TRACE("DirectDrawClipper::~DirectDrawClipper");
}

/*** IUnknown methods ***/
HRESULT WINAPI DirectDrawClipper::QueryInterface(REFIID riid, LPVOID* ppvObj) {
    TRACE("DirectDrawClipper::QueryInterface");
    if (IsEqualGUID(riid, IID_IDirectDrawSurface)) {
        *ppvObj = static_cast<IDirectDrawClipper*>(this);
    } else {
        return Unknown::QueryInterface(riid, ppvObj);
    }

    Unknown::AddRef();
    return S_OK;
}

ULONG WINAPI DirectDrawClipper::AddRef() {
    return Unknown::AddRef();
}

ULONG WINAPI DirectDrawClipper::Release() {
    return Unknown::Release();
}

/*** IDirectDrawClipper methods ***/
HRESULT WINAPI DirectDrawClipper::GetClipList(LPRECT lpRect, LPRGNDATA lpClipList, LPDWORD lpdwSize) {
    TRACE("DirectDrawClipper::GetClipList");
    *lpClipList = m_clipList;
    return DD_OK;
}

HRESULT WINAPI DirectDrawClipper::GetHWnd(HWND* lphWnd) {
    TRACE("DirectDrawClipper::GetHWnd");
    *lphWnd = m_hWnd;
    return DD_OK;
}

HRESULT WINAPI DirectDrawClipper::Initialize(LPDIRECTDRAW lpDD, DWORD dwFlags) {
    TRACE("DirectDrawClipper::Initialize");
    return DD_OK;
}

HRESULT WINAPI DirectDrawClipper::IsClipListChanged(BOOL* lpbChanged) {
    TRACE("DirectDrawClipper::IsClipListChanged");
    *lpbChanged = FALSE;
    return DD_OK;
}

HRESULT WINAPI DirectDrawClipper::SetClipList(LPRGNDATA lpClipList, DWORD dwFlags) {
    TRACE("DirectDrawClipper::SetClipList");
    m_clipList = *lpClipList;
    return DD_OK;
}

HRESULT WINAPI DirectDrawClipper::SetHWnd(DWORD dwFlags, HWND hWnd) {
    TRACE("DirectDrawClipper::SetHWnd");
    m_hWnd = hWnd;
    return DD_OK;
}

}