#include "Screenshot.hpp"

#include "gl_core_3_3.h"
#include "StringUtils.hpp"
#include "ErrorUtils.hpp"
#include "GLRage.hpp"

#include <stdexcept>
#include <exception>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

namespace glrage {

// heavily simplified Targa header struct for raw BGR(A) data
struct MiniTargaHeader {
    uint8_t blank1[2];
    uint8_t format;
    uint8_t blank2[9];
    uint16_t width;
    uint16_t height;
    uint8_t depth;
    uint8_t blank3;
};

void Screenshot::schedule(bool schedule) {
    m_schedule = schedule;
}

void Screenshot::captureScheduled() {
    if (m_schedule) {
        capture();
        m_schedule = false;
    }
}

void Screenshot::capture() {
    // find unused screenshot file name
    std::wstring basePath = GLRageGetContext().getBasePath();
    std::wstring path;
    DWORD dwAttrib;
    do {
        std::string fileName = StringUtils::format("screenshot%04d.tga", 20, m_index++);
        path = basePath + L"\\" + StringUtils::utf8ToWide(fileName);
        dwAttrib = GetFileAttributes(path.c_str());
    } while (dwAttrib != INVALID_FILE_ATTRIBUTES && m_index < 9999);

    // rather unlikely, but better safe than sorry
    if (dwAttrib != INVALID_FILE_ATTRIBUTES) {
        throw std::runtime_error("All available screenshot slots are used up!");
    }

    // open screenshot file
    std::ofstream file(path, std::ofstream::binary);
    if (!file.good()) {
        throw std::runtime_error("Can't open screenshot file '" + StringUtils::wideToUtf8(path) + "': " +
            ErrorUtils::getSystemErrorString());
    }

    // get viewport dimension
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport); 

    uint32_t x = viewport[0];
    uint32_t y = viewport[1];
    uint32_t width = viewport[2];
    uint32_t height = viewport[3];
    uint8_t depth = 3;

    // copy framebuffer to local buffer
    std::vector<uint8_t> data(width * height * depth);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(x, y, width, height, GL_BGR, GL_UNSIGNED_BYTE, &data[0]);

    // create Targa header
    MiniTargaHeader tgaHeader = {0};
    tgaHeader.format = 2;
    tgaHeader.width = width;
    tgaHeader.height = height;
    tgaHeader.depth = depth * 8;

    file.write(reinterpret_cast<char*>(&tgaHeader), sizeof(MiniTargaHeader));
    file.write(reinterpret_cast<char*>(&data[0]), data.size());
    file.close();
}

}