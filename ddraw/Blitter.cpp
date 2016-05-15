#include "Blitter.hpp"

#include <algorithm>

namespace glrage {
namespace ddraw {

void Blitter::blit(Image& srcImg, Rect& srcRect, Image dstImg, Rect& dstRect)
{
    // do fast direct copy if possible
    if (srcImg == dstImg && srcRect == dstRect) {
        dstImg.buffer = srcImg.buffer;
        return;
    }

    int32_t srcRectWidth = srcRect.width();
    int32_t srcRectHeight = srcRect.height();

    int32_t dstRectWidth = dstRect.width();
    int32_t dstRectHeight = dstRect.height();

    int32_t xRatio = ((srcRectWidth << m_ratioBias) / dstRectWidth) + 1;
    int32_t yRatio = ((srcRectHeight << m_ratioBias) / dstRectHeight) + 1;

    for (int32_t y = 0; y < dstRectHeight; y++) {
        int32_t y1 = dstRect.top + y;
        int32_t y2 = srcRect.top + ((y * yRatio) >> m_ratioBias);

        for (int32_t x = 0; x < dstRectWidth; x++) {
            int32_t x1 = dstRect.left + x;
            int32_t x2 = srcRect.left + ((x * xRatio) >> m_ratioBias);

            for (int32_t n = 0; n < dstImg.depth; n++) {
                dstImg(x1, y1, n) = srcImg(x2, y2, n);
            }
        }
    }
}

} // namespace ddraw
} // namespace glrage