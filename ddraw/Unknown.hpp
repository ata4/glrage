#pragma once

#include <Windows.h>
#include <Unknwnbase.h>

namespace ddraw {

class Unknown : public IUnknown {
public:
    Unknown();
    virtual ~Unknown();

    virtual HRESULT WINAPI QueryInterface(REFIID riid, LPVOID* ppvObj);
    virtual ULONG WINAPI AddRef();
    virtual ULONG WINAPI Release();

private:
    volatile ULONG m_refCount;
};

}