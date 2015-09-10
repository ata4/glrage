#pragma once

#include "ati3dcif.h"

#include "VertexArray.hpp"
#include "VertexArrayBuffer.hpp"

#include <vector>

using glrage::VertexArrayBuffer;
using glrage::VertexArray;

namespace cif {

class CifVertexStream {
public:
    CifVertexStream();
    void uploadPrimStrip(C3D_VSTRIP vertStrip, C3D_UINT32 numVert);
    void uploadPrimList(C3D_VLIST vertList, C3D_UINT32 numVert);
    C3D_EVERTEX vertexType();
    void vertexType(C3D_EVERTEX vertexType);
    void bind();

private:
    void reserve(C3D_UINT32 numVert, size_t bufferSize);

    C3D_EVERTEX m_vertexType;
    size_t m_maxVert;
    size_t m_vertexBufferSize;
    VertexArrayBuffer m_vertexBuffer;
    VertexArray m_vtcFormat;
    std::vector<C3D_VTCF> m_vtcBuffer;
};

}