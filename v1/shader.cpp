#include "shader.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader(EngineObject* parent)
 : EngineObject{parent, "Shader"}
{
}

Shader::~Shader()
{
    if (!m_freed)
    {
        Shader::free();
    }
}

void Shader::free()
{
    EngineObject::free();
}

void Shader::loadFromFile(const char* fragPath, const char* vertPath)
{
    std::string vertCode;
    std::string fragCode;
    std::ifstream vertFile;
    std::ifstream fragFile;

    vertFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    // load shader source from file
    try
    {
        vertFile.open(vertPath);
        fragFile.open(fragPath);
        std::stringstream vertStream, fragStream;

        vertStream << vertFile.rdbuf();
        fragStream << fragFile.rdbuf();

        vertFile.close();
        fragFile.close();

        vertCode = vertStream.str();
        fragCode = fragStream.str();
    } catch ([[maybe_unused]] std::ifstream::failure& e)
    {
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Could not read source files: {vert: `" << vertPath << "`, frag: `" << fragPath << "`}" << std::endl;
        return;
    }

    const char* vShaderCode {vertCode.c_str()};
    const char* fShaderCode {fragCode.c_str()};

    // compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512]; // for errors

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Vertex shader compilation failed." << std::endl << infoLog;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Fragment shader compilation failed." << std::endl << infoLog;
    }

    // actually create the program
    unsigned int id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    // get linking errors
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Shader linking failed." << std::endl << infoLog;
    }

    // update m_ID
    m_ID = id;

    // no longer needed
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    std::cout << "Loaded shader from files: `" << vertPath << "` `" << fragPath << "`\n";
}

void Shader::use() const
{
    glUseProgram(m_ID);
}

unsigned int Shader::getID() const
{
    return m_ID;
}


// ----- shader uniform setters -----
void Shader::setBool(const std::string& name, const bool value) const
{
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, const int value) const
{
    glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, const float value) const
{
    glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value);
}


// vectorz
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string& name, const float x, const float y) const
{
    glUniform2f(glGetUniformLocation(m_ID, name.c_str()), x, y);
}

// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const float x, const float y, const float z) const
{
    glUniform3f(glGetUniformLocation(m_ID, name.c_str()), x, y, z);
}

// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(m_ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const float x, const float y, const float z, const float w) const
{
    glUniform4f(glGetUniformLocation(m_ID, name.c_str()), x, y, z, w);
}


// matrices
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& value) const
{
    glUniformMatrix2fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
