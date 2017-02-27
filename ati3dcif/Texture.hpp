#pragma once

#include "ati3dcif.hpp"

#include <vector>

#include <glrage/GLRage.hpp>
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
    Config& m_config{GLRage::getConfig()};
    C3D_COLOR m_chromaKey;
    std::map<std::string, std::string>& getTextureKeys();
};

} // namespace cif
} // namespace glrage