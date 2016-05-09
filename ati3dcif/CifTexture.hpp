#pragma once

#include "ati3dcif.h"

#include "Texture.hpp"

using glrage::Texture;

namespace cif {

class CifTexture : public Texture
{
public:
    CifTexture();
    ~CifTexture();
    void load(C3D_PTMAP tmap, C3D_PPALETTENTRY palette);
    C3D_COLOR& chromaKey();

private:
    C3D_COLOR m_chromaKey;
};

} // namespace cif