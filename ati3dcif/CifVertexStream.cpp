#include "CifVertexStream.hpp"
#include "CifUnimplementedException.hpp"
#include "CifUtils.hpp"

#include "Logger.hpp"
#include "GLUtils.hpp"

using glrage::GLUtils;

namespace cif {

CifVertexStream::CifVertexStream() {
    m_maxVert = 0;
    m_vertexBufferSize = 0;

    // bind vertex buffer
    m_vertexBuffer.bind();

    // define vertex formats
    m_vtcFormat.bind();
    m_vtcFormat.attribute(0, 3, GL_FLOAT, GL_FALSE, 40, 0);
    m_vtcFormat.attribute(1, 3, GL_FLOAT, GL_FALSE, 40, 12);
    m_vtcFormat.attribute(2, 4, GL_FLOAT, GL_FALSE, 40, 24);

    GLUtils::checkError("CifVertexStream::CifVertexStream");
}

void CifVertexStream::uploadPrimStrip(C3D_VSTRIP vertStrip, C3D_UINT32 numVert) {
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
            throw CifUnimplementedException("Unsupported vertex type: " + std::string(C3D_EVERTEX_NAMES[m_vertexType]));
    }

    GLUtils::checkError("CifVertexStream::uploadPrimStrip");
}

void CifVertexStream::uploadPrimList(C3D_VLIST vertList, C3D_UINT32 numVert) {
    switch (m_vertexType) {
        case C3D_EV_VTCF: {
            // bind vertex format
            m_vtcFormat.bind();

            // resize vertex buffer if required
            reserve(numVert, sizeof(C3D_VTCF));

            // copy vertices to vertex vector buffer, then to the vertex buffer
            // (OpenGL can't handle arrays of pointers)
            C3D_VTCF** vListVtcf = reinterpret_cast<C3D_VTCF**>(vertList);
            for (C3D_UINT32 i = 0; i < numVert; i++) {
                m_vtcBuffer[i] = *vListVtcf[i];
            }

            m_vertexBuffer.subData(0, sizeof(C3D_VTCF) * numVert, &m_vtcBuffer[0]);

            break;
        }

        default:
            throw CifUnimplementedException("Unsupported vertex type: " + std::string(C3D_EVERTEX_NAMES[m_vertexType]));
    }

    GLUtils::checkError("CifVertexStream::uploadPrimList");
}

C3D_EVERTEX CifVertexStream::vertexType() {
    return m_vertexType;
}

void CifVertexStream::vertexType(C3D_EVERTEX vertexType) {
    m_vertexType = vertexType;
}

void CifVertexStream::reserve(C3D_UINT32 numVert, size_t vertSize) {
    if (numVert > m_maxVert) {
        LOGF("CifVertexStream::reserve: Vertex vector resize: %d -> %d", m_maxVert, numVert);
        m_vtcBuffer.resize(numVert);
        m_maxVert = numVert;
    }

    size_t vertexBufferSize = vertSize * numVert;
    if (vertexBufferSize > m_vertexBufferSize) {
        LOGF("CifVertexStream::reserve: Vertex buffer resize: %d -> %d", m_vertexBufferSize, vertexBufferSize);
        m_vertexBuffer.data(vertexBufferSize, nullptr, GL_STREAM_DRAW);
        m_vertexBufferSize = vertexBufferSize;
    }
}

void CifVertexStream::bind() {
    m_vertexBuffer.bind();
}

}