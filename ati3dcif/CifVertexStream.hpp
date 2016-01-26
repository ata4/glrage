#pragma once

#include "ati3dcif.h"

#include "VertexArray.hpp"
#include "VertexArrayBuffer.hpp"

#include <vector>

using glrage::VertexArrayBuffer;
using glrage::VertexArray;

namespace cif {

// for RenderPrimList
static const GLenum GLCIF_PRIM_MODES[] = {
    GL_LINES,           // C3D_EPRIM_LINE
    GL_TRIANGLES,       // C3D_EPRIM_TRI
    GL_TRIANGLES,       // C3D_EPRIM_QUAD
    GL_TRIANGLES,       // C3D_EPRIM_RECT
    GL_POINTS           // C3D_EPRIM_POINT
};

// for RenderPrimStrip
static const GLenum GLCIF_PRIMSTRIP_MODES[] = {
    GL_LINE_STRIP,      // C3D_EPRIM_LINE
    GL_TRIANGLE_STRIP,  // C3D_EPRIM_TRI
    GL_TRIANGLE_STRIP,  // C3D_EPRIM_QUAD
    GL_TRIANGLE_STRIP,  // C3D_EPRIM_RECT
    GL_POINTS           // C3D_EPRIM_POINT
};

class CifVertexStream {
public:
    CifVertexStream();
    void renderPrimStrip(C3D_VSTRIP vertStrip, C3D_UINT32 numVert);
    void renderPrimList(C3D_VLIST vertList, C3D_UINT32 numVert);
    C3D_EVERTEX vertexType();
    void vertexType(C3D_EVERTEX vertexType);
    C3D_EPRIM primType();
    void primType(C3D_EPRIM primType);
    void bind();

private:
    void reserve(C3D_UINT32 numVert, size_t bufferSize);

    C3D_EVERTEX m_vertexType;
    C3D_EPRIM m_primType;
    size_t m_vertexBufferSize = 0;
    VertexArrayBuffer m_vertexBuffer;
    VertexArray m_vtcFormat;
    std::vector<C3D_VTCF> m_vtcBuffer;
};

}