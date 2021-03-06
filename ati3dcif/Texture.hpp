#pragma once

#include "ati3dcif.hpp"

#include <vector>

#include <glrage_gl/Texture.hpp>

namespace glrage {
namespace cif {

class Texture : public gl::Texture
{
public:
    Texture();
    ~Texture();
    void load(C3D_PTMAP tmap, std::vector<C3D_PALETTENTRY>& palette);
    C3D_COLOR& chromaKey();

private:
    C3D_COLOR m_chromaKey;
};

} // namespace cif
} // namespace glrage