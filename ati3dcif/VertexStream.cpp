#include "VertexStream.hpp"
#include "Error.hpp"
#include "Utils.hpp"

#include <glrage_gl/Utils.hpp>
#include <glrage_util/Logger.hpp>

namespace glrage {
namespace cif {

VertexStream::VertexStream()
{
    // bind vertex buffer
    m_vertexBuffer.bind();

    // define vertex formats
    m_vtcFormat.bind();
    m_vtcFormat.attribute(0, 3, GL_FLOAT, GL_FALSE, 40, 0);
    m_vtcFormat.attribute(1, 3, GL_FLOAT, GL_FALSE, 40, 12);
    m_vtcFormat.attribute(2, 4, GL_FLOAT, GL_FALSE, 40, 24);

    gl::Utils::checkError("VertexStream::VertexStream");
}

void VertexStream::renderPrimStrip(C3D_VSTRIP vertStrip, C3D_UINT32 numVert)
{
    switch (m_vertexType) {
        case C3D_EV_VTCF: {
            // bind vertex format
            m_vtcFormat.bind();

            // resize vertex buffer if required
            reserve(numVert, sizeof(C3D_VTCF));

            // copy vertices dírectly to vertex buffer
            C3D_VTCF* vStripVtcf = reinterpret_cast<C3D_VTCF*>(vertStrip);
            m_vertexBuffer.subData(0, sizeof(C3D_VTCF) * numVert, vStripVtcf);

            break;
        }

        default:
            throw Error("Unsupported vertex type: " +
                               std::string(C3D_EVERTEX_NAMES[m_vertexType]),
                C3D_EC_NOTIMPYET);
    }

    glDrawArrays(GLCIF_PRIMSTRIP_MODES[m_primType], 0, numVert);

    gl::Utils::checkError("VertexStream::renderPrimStrip");
}

void VertexStream::renderPrimList(C3D_VLIST vertList, C3D_UINT32 numVert)
{
    switch (m_vertexType) {
        case C3D_EV_VTCF: {
            // bind vertex format
            m_vtcFormat.bind();

            // copy vertices to vertex vector buffer, then to the vertex buffer
            // (OpenGL can't handle arrays of pointers)
            C3D_VTCF** vListVtcf = reinterpret_cast<C3D_VTCF**>(vertList);
            m_vtcBuffer.clear();

            if (m_primType == C3D_EPRIM_QUAD) {
                // triangulate quads
                for (C3D_UINT32 i = 0; i < numVert; i += 4) {
                    m_vtcBuffer.push_back(*vListVtcf[i + 0]);
                    m_vtcBuffer.push_back(*vListVtcf[i + 1]);
                    m_vtcBuffer.push_back(*vListVtcf[i + 3]);

                    m_vtcBuffer.push_back(*vListVtcf[i + 1]);
                    m_vtcBuffer.push_back(*vListVtcf[i + 2]);
                    m_vtcBuffer.push_back(*vListVtcf[i + 3]);
                }

                numVert = m_vtcBuffer.size();
            } else {
                // direct copy
                for (C3D_UINT32 i = 0; i < numVert; i++) {
                    m_vtcBuffer.push_back(*vListVtcf[i]);
                }
            }

            // resize vertex buffer if required
            reserve(numVert, sizeof(C3D_VTCF));

            // upload vertex buffer
            m_vertexBuffer.subData(
                0, sizeof(C3D_VTCF) * numVert, &m_vtcBuffer[0]);

            break;
        }

        default:
            throw Error("Unsupported vertex type: " +
                               std::string(C3D_EVERTEX_NAMES[m_vertexType]),
                C3D_EC_NOTIMPYET);
    }

    glDrawArrays(GLCIF_PRIM_MODES[m_primType], 0, numVert);

    gl::Utils::checkError("VertexStream::renderPrimList");
}

C3D_EVERTEX
VertexStream::vertexType()
{
    return m_vertexType;
}

C3D_EPRIM
VertexStream::primType()
{
    return m_primType;
}

void VertexStream::primType(C3D_EPRIM primType)
{
    m_primType = primType;
}

void VertexStream::vertexType(C3D_EVERTEX vertexType)
{
    m_vertexType = vertexType;
}

void VertexStream::reserve(C3D_UINT32 numVert, size_t vertSize)
{
    size_t vertexBufferSize = vertSize * numVert;
    if (vertexBufferSize > m_vertexBufferSize) {
        LOG_INFO("Vertex buffer resize: %d -> %d", m_vertexBufferSize,
            vertexBufferSize);
        m_vertexBuffer.data(vertexBufferSize, nullptr, GL_STREAM_DRAW);
        m_vertexBufferSize = vertexBufferSize;
    }
}

void VertexStream::bind()
{
    m_vertexBuffer.bind();
}

} // namespace cif
} // namespace glrage