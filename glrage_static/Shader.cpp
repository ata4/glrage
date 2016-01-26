#include "Shader.hpp"
#include "ShaderException.hpp"
#include "StringUtils.hpp"
#include "ErrorUtils.hpp"

#include <string>
#include <fstream>
#include <sstream>

namespace glrage {

Shader::Shader(GLenum type) {
    m_id = glCreateShader(type);
    if (!m_id) {
        throw ShaderException("Can't create shader");
    }
}

Shader::~Shader() {
    if (m_id) {
        glDeleteShader(m_id);
    }
}

void Shader::bind() {
}

Shader& Shader::fromFile(const std::wstring& path) {
    // open and check shader file
    std::ifstream file;
    file.open(path.c_str());
    if (!file.good()) {
        throw std::runtime_error("Can't open shader file '" +
            StringUtils::wideToUtf8(path) + "': " +
            ErrorUtils::getSystemErrorString());
    }

    // read file to a string stream
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();

    // convert stream to string
    fromString(stream.str());

    return *this;
}

Shader& Shader::fromString(const std::string& program) {
    // create shader source
    const char *programChars = program.c_str();
    glShaderSource(m_id, 1, &programChars, nullptr);

    // compile the shader
    glCompileShader(m_id);

    // check the compilation status and throw exception if shader compilation failed
    if (!compiled()) {
        std::string message = infoLog();
        if (message.empty()) {
            message = "Shader compilation failed.";
        }
        throw ShaderException(message);
    }

    return *this;
}

std::string Shader::infoLog() {
    // get length and data of info log entry
    GLint infoLogLength;
    glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (!infoLogLength) {
        return "";
    }

    GLchar *infoLogChars = new GLchar[infoLogLength + 1];
    glGetShaderInfoLog(m_id, infoLogLength, nullptr, infoLogChars);

    // convert the info log to a string
    std::string infoLogString(infoLogChars);

    // delete the char array version of the log
    delete[] infoLogChars;

    // finally, return the string version of the info log
    return infoLogString;
}

bool Shader::compiled() {
    int compileStatus;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &compileStatus);
    return compileStatus == GL_TRUE;
}

}