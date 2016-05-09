#include "SurfaceDebugUtils.hpp"

#include "ErrorUtils.hpp"
#include "Logger.hpp"
#include "StringUtils.hpp"

#include <cstdint>
#include <exception>
#include <fstream>
#include <vector>

using glrage::StringUtils;
using glrage::ErrorUtils;

namespace ddraw {

void SurfaceDebugUtils::dumpInfo(DDSURFACEDESC& desc)
{
#ifdef DEBUG_LOG
    LOG("SurfaceDebugUtils::dumpInfo start");
    LOGF("  desc.dwWidth = %d", desc.dwWidth);
    LOGF("  desc.dwHeight = %d", desc.dwHeight);
    LOGF("  desc.ddpfPixelFormat.dwRGBBitCount = %d",
        desc.ddpfPixelFormat.dwRGBBitCount);

    LOGF("  desc.dwFlags = %d", desc.dwFlags);
    if (desc.dwFlags & DDSD_CAPS)
        LOG("    DDSD_CAPS");
    if (desc.dwFlags & DDSD_HEIGHT)
        LOG("    DDSD_HEIGHT");
    if (desc.dwFlags & DDSD_WIDTH)
        LOG("    DDSD_WIDTH");
    if (desc.dwFlags & DDSD_PITCH)
        LOG("    DDSD_PITCH");
    if (desc.dwFlags & DDSD_BACKBUFFERCOUNT)
        LOG("    DDSD_BACKBUFFERCOUNT");
    if (desc.dwFlags & DDSD_ZBUFFERBITDEPTH)
        LOG("    DDSD_ZBUFFERBITDEPTH");
    if (desc.dwFlags & DDSD_ALPHABITDEPTH)
        LOG("    DDSD_ALPHABITDEPTH");
    if (desc.dwFlags & DDSD_LPSURFACE)
        LOG("    DDSD_LPSURFACE");
    if (desc.dwFlags & DDSD_PIXELFORMAT)
        LOG("    DDSD_PIXELFORMAT");
    if (desc.dwFlags & DDSD_CKDESTOVERLAY)
        LOG("    DDSD_CKDESTOVERLAY");
    if (desc.dwFlags & DDSD_CKDESTBLT)
        LOG("    DDSD_CKDESTBLT");
    if (desc.dwFlags & DDSD_CKSRCOVERLAY)
        LOG("    DDSD_CKSRCOVERLAY");
    if (desc.dwFlags & DDSD_CKSRCBLT)
        LOG("    DDSD_CKSRCBLT");
    if (desc.dwFlags & DDSD_MIPMAPCOUNT)
        LOG("    DDSD_MIPMAPCOUNT");
    if (desc.dwFlags & DDSD_REFRESHRATE)
        LOG("    DDSD_REFRESHRATE");
    if (desc.dwFlags & DDSD_LINEARSIZE)
        LOG("    DDSD_LINEARSIZE");
    if (desc.dwFlags & DDSD_TEXTURESTAGE)
        LOG("    DDSD_TEXTURESTAGE");
    if (desc.dwFlags & DDSD_FVF)
        LOG("    DDSD_FVF");
    if (desc.dwFlags & DDSD_SRCVBHANDLE)
        LOG("    DDSD_SRCVBHANDLE");
    if (desc.dwFlags & DDSD_DEPTH)
        LOG("    DDSD_DEPTH");

    LOGF("  desc.ddsCaps.dwCaps = %d", desc.ddsCaps.dwCaps);
    if (desc.ddsCaps.dwCaps & DDSCAPS_RESERVED1)
        LOG("    DDSCAPS_RESERVED1");
    if (desc.ddsCaps.dwCaps & DDSCAPS_ALPHA)
        LOG("    DDSCAPS_ALPHA");
    if (desc.ddsCaps.dwCaps & DDSCAPS_BACKBUFFER)
        LOG("    DDSCAPS_BACKBUFFER");
    if (desc.ddsCaps.dwCaps & DDSCAPS_COMPLEX)
        LOG("    DDSCAPS_COMPLEX");
    if (desc.ddsCaps.dwCaps & DDSCAPS_FLIP)
        LOG("    DDSCAPS_FLIP");
    if (desc.ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER)
        LOG("    DDSCAPS_FRONTBUFFER");
    if (desc.ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN)
        LOG("    DDSCAPS_OFFSCREENPLAIN");
    if (desc.ddsCaps.dwCaps & DDSCAPS_OVERLAY)
        LOG("    DDSCAPS_OVERLAY");
    if (desc.ddsCaps.dwCaps & DDSCAPS_PALETTE)
        LOG("    DDSCAPS_PALETTE");
    if (desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
        LOG("    DDSCAPS_PRIMARYSURFACE");
    if (desc.ddsCaps.dwCaps & DDSCAPS_RESERVED3)
        LOG("    DDSCAPS_RESERVED3");
    if (desc.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACELEFT)
        LOG("    DDSCAPS_PRIMARYSURFACELEFT");
    if (desc.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
        LOG("    DDSCAPS_SYSTEMMEMORY");
    if (desc.ddsCaps.dwCaps & DDSCAPS_TEXTURE)
        LOG("    DDSCAPS_TEXTURE");
    if (desc.ddsCaps.dwCaps & DDSCAPS_3DDEVICE)
        LOG("    DDSCAPS_3DDEVICE");
    if (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
        LOG("    DDSCAPS_VIDEOMEMORY");
    if (desc.ddsCaps.dwCaps & DDSCAPS_VISIBLE)
        LOG("    DDSCAPS_VISIBLE");
    if (desc.ddsCaps.dwCaps & DDSCAPS_WRITEONLY)
        LOG("    DDSCAPS_WRITEONLY");
    if (desc.ddsCaps.dwCaps & DDSCAPS_ZBUFFER)
        LOG("    DDSCAPS_ZBUFFER");
    if (desc.ddsCaps.dwCaps & DDSCAPS_OWNDC)
        LOG("    DDSCAPS_OWNDC");
    if (desc.ddsCaps.dwCaps & DDSCAPS_LIVEVIDEO)
        LOG("    DDSCAPS_LIVEVIDEO");
    if (desc.ddsCaps.dwCaps & DDSCAPS_HWCODEC)
        LOG("    DDSCAPS_HWCODEC");
    if (desc.ddsCaps.dwCaps & DDSCAPS_MODEX)
        LOG("    DDSCAPS_MODEX");
    if (desc.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
        LOG("    DDSCAPS_MIPMAP");
    if (desc.ddsCaps.dwCaps & DDSCAPS_RESERVED2)
        LOG("    DDSCAPS_RESERVED2");
    if (desc.ddsCaps.dwCaps & DDSCAPS_ALLOCONLOAD)
        LOG("    DDSCAPS_ALLOCONLOAD");
    if (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOPORT)
        LOG("    DDSCAPS_VIDEOPORT");
    if (desc.ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
        LOG("    DDSCAPS_LOCALVIDMEM");
    if (desc.ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        LOG("    DDSCAPS_NONLOCALVIDMEM");
    if (desc.ddsCaps.dwCaps & DDSCAPS_STANDARDVGAMODE)
        LOG("    DDSCAPS_STANDARDVGAMODE");
    if (desc.ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
        LOG("    DDSCAPS_OPTIMIZED");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_RESERVED4) LOG(" DDSCAPS2_RESERVED4");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_HARDWAREDEINTERLACE) LOG("
    // DDSCAPS2_HARDWAREDEINTERLACE");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_HINTDYNAMIC) LOG("
    // DDSCAPS2_HINTDYNAMIC");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_HINTSTATIC) LOG("
    // DDSCAPS2_HINTSTATIC");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_TEXTUREMANAGE) LOG("
    // DDSCAPS2_TEXTUREMANAGE");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_RESERVED1) LOG(" DDSCAPS2_RESERVED1");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_RESERVED2) LOG(" DDSCAPS2_RESERVED2");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_OPAQUE) LOG("    DDSCAPS2_OPAQUE");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_HINTANTIALIASING) LOG("
    // DDSCAPS2_HINTANTIALIASING");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP) LOG("    DDSCAPS2_CUBEMAP");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP_POSITIVEX) LOG("
    // DDSCAPS2_CUBEMAP_POSITIVEX");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP_NEGATIVEX) LOG("
    // DDSCAPS2_CUBEMAP_NEGATIVEX");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP_POSITIVEY) LOG("
    // DDSCAPS2_CUBEMAP_POSITIVEY");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP_NEGATIVEY) LOG("
    // DDSCAPS2_CUBEMAP_NEGATIVEY");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP_POSITIVEZ) LOG("
    // DDSCAPS2_CUBEMAP_POSITIVEZ");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_CUBEMAP_NEGATIVEZ) LOG("
    // DDSCAPS2_CUBEMAP_NEGATIVEZ");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_MIPMAPSUBLEVEL) LOG("
    // DDSCAPS2_MIPMAPSUBLEVEL");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_D3DTEXTUREMANAGE) LOG("
    // DDSCAPS2_D3DTEXTUREMANAGE");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_DONOTPERSIST) LOG("
    // DDSCAPS2_DONOTPERSIST");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_STEREOSURFACELEFT) LOG("
    // DDSCAPS2_STEREOSURFACELEFT");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_VOLUME) LOG("    DDSCAPS2_VOLUME");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_NOTUSERLOCKABLE) LOG("
    // DDSCAPS2_NOTUSERLOCKABLE");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_POINTS) LOG("    DDSCAPS2_POINTS");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_RTPATCHES) LOG(" DDSCAPS2_RTPATCHES");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_NPATCHES) LOG(" DDSCAPS2_NPATCHES");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_RESERVED3) LOG(" DDSCAPS2_RESERVED3");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_DISCARDBACKBUFFER) LOG("
    // DDSCAPS2_DISCARDBACKBUFFER");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_ENABLEALPHACHANNEL) LOG("
    // DDSCAPS2_ENABLEALPHACHANNEL");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_EXTENDEDFORMATPRIMARY) LOG("
    // DDSCAPS2_EXTENDEDFORMATPRIMARY");
    // if (desc.ddsCaps.dwCaps & DDSCAPS2_ADDITIONALPRIMARY) LOG("
    // DDSCAPS2_ADDITIONALPRIMARY");
    LOG("SurfaceDebugUtils::dumpInfo end");
#endif
}

void SurfaceDebugUtils::dumpBuffer(
    DDSURFACEDESC& desc, void* buffer, const std::wstring& path)
{
    uint32_t imageSize = desc.dwWidth * desc.dwHeight;
    uint32_t dataSize = imageSize * 3;
    uint32_t headerSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    uint16_t* src = reinterpret_cast<uint16_t*>(buffer);
    std::vector<uint8_t> dst(dataSize);

    for (uint32_t i = 0; i < imageSize; i++) {
        dst[i * 3 + 0] = (src[i] >> 0 & 0x1f) * 0xff / 0x1f;
        dst[i * 3 + 1] = (src[i] >> 5 & 0x1f) * 0xff / 0x1f;
        dst[i * 3 + 2] = (src[i] >> 10 & 0x1f) * 0xff / 0x1f;
    }

    BITMAPFILEHEADER fh;
    fh.bfType = 0x4d42;
    fh.bfSize = dataSize + headerSize;
    fh.bfReserved1 = 0;
    fh.bfReserved2 = 0;
    fh.bfOffBits = headerSize;

    BITMAPINFOHEADER ih;
    ih.biSize = sizeof(BITMAPINFOHEADER);
    ih.biWidth = desc.dwWidth;
    ih.biHeight = desc.dwHeight;
    ih.biPlanes = 1;
    ih.biBitCount = 24;
    ih.biCompression = 0;
    ih.biSizeImage = 0;
    ih.biXPelsPerMeter = 2835;
    ih.biYPelsPerMeter = 2835;
    ih.biClrUsed = 0;
    ih.biClrImportant = 0;

    std::ofstream file(path, std::ofstream::binary);
    if (!file.good()) {
        throw std::runtime_error("Can't open file '" +
                                 StringUtils::wideToUtf8(path) + "': " +
                                 ErrorUtils::getSystemErrorString());
    }

    file.write(reinterpret_cast<char*>(dst[0]), dataSize);
    file.close();
}

std::string SurfaceDebugUtils::getSurfaceName(DDSURFACEDESC& desc)
{
    if (desc.dwFlags & DDSCAPS_PRIMARYSURFACE) {
        return "Primary";
    } else if (desc.dwFlags & DDSCAPS_FRONTBUFFER) {
        return "Front";
    } else if (desc.dwFlags & DDSCAPS_BACKBUFFER) {
        return "Back";
    } else {
        return "Other";
    }
}

} // namespace ddraw