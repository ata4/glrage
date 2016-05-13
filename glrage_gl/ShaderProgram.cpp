#include "ShaderProgram.hpp"
#include "Logger.hpp"
#include "ShaderProgramException.hpp"

namespace glrage {

ShaderProgram::ShaderProgram()
{
    m_id = glCreateProgram();
    if (!m_id) {
        throw ShaderProgramException("Can't create shader program");
    }
}

ShaderProgram::~ShaderProgram()
{
    if (m_id) {
        glDeleteProgram(m_id);
    }
}

void ShaderProgram::bind()
{
    glUseProgram(m_id);
}

void ShaderProgram::attach(Shader& shader)
{
    glAttachShader(m_id, shader.id());
}

void ShaderProgram::detach(Shader& shader)
{
    glDetachShader(m_id, shader.id());
}

void ShaderProgram::link()
{
    // do the linking
    glLinkProgram(m_id);

    // check for linking errors
    GLint linkStatus;
    glGetProgramiv(m_id, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus) {
        std::string message = infoLog();
        if (message.empty()) {
            message = "Shader linking failed.";
        }
        throw ShaderProgramException(message);
    }
}

void ShaderProgram::fragmentData(const std::string& name)
{
    glBindFragDataLocation(m_id, 0, name.c_str());
}

GLint ShaderProgram::attributeLocation(const std::string& name)
{
    auto it = m_attributeLocations.find(name);
    if (it != m_attributeLocations.end()) {
        return it->second;
    }

    GLint location = glGetAttribLocation(m_id, name.c_str());
    if (location == -1) {
        LOG_INFO("Shader attribute not found: " + name);
    }

    m_attributeLocations[name] = location;

    return location;
}

GLint ShaderProgram::uniformLocation(const std::string& name)
{
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    GLint location = glGetUniformLocation(m_id, name.c_str());
    if (location == -1) {
        LOG_INFO("Shader uniform not found: " + name);
    }

    m_uniformLocations[name] = location;

    return location;
}

void ShaderProgram::uniform3f(
    const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    GLint loc = uniformLocation(name);
    if (loc != -1) {
        glUniform3f(loc, v0, v1, v2);
    }
}

void ShaderProgram::uniform4f(
    const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GLint loc = uniformLocation(name);
    if (loc != -1) {
        glUniform4f(loc, v0, v1, v2, v3);
    }
}

void ShaderProgram::uniform1i(const std::string& name, GLint v0)
{
    GLint loc = uniformLocation(name);
    if (loc != -1) {
        glUniform1i(loc, v0);
    }
}

void ShaderProgram::uniformMatrix4fv(const std::string& name, GLsizei count,
    GLboolean transpose, const GLfloat* value)
{
    GLint loc = uniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, count, transpose, value);
    }
}

std::string ShaderProgram::infoLog()
{
    GLint infoLogLength;
    std::string infoLogString;
    infoLogString.resize(4096);
    glGetProgramInfoLog(m_id, infoLogString.capacity(), &infoLogLength, &infoLogString[0]);
    infoLogString.resize(infoLogLength);
    return infoLogString;
}

} // namespace glrage