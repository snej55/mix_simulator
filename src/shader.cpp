#include <glad/glad.h>

#include "shader.hpp"
#include "util.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


Shader::Shader(const std::string& name, EngineObject* parent) :
    EngineObject{("SHADER " + name).c_str(), parent}, m_shaderName{name}
{
}

Shader::~Shader() { glDeleteProgram(m_ID); }

bool Shader::loadFromFile(const char* fragPath, const char* vertPath)
{
    bool shaderSuccess{true};

    // check if shader files exist
    if (!Util::fileExists(fragPath))
    {
        Util::beginError();
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Failed to read fragment shader from `" << fragPath
                  << "` - file does not exist!";
        Util::endError();
        return false;
    }
    else if (!Util::fileExists(vertPath))
    {
        Util::beginError();
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Failed to read vertex shader from `" << vertPath
                  << "` - file does not exist!";
        Util::endError();
        return false;
    }

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
    }
    catch ([[maybe_unused]] std::ifstream::failure& e)
    {
        Util::beginError();
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Could not read source files: {vert: `" << vertPath << "`, frag: `"
                  << fragPath << "`}";
        Util::endError();
        return false;
    }

    const char* vShaderCode{vertCode.c_str()};
    const char* fShaderCode{fragCode.c_str()};

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
        Util::beginError();
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Vertex shader compilation failed." << std::endl << infoLog;
        Util::endError();
        shaderSuccess = false;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        Util::beginError();
        glGetShaderInfoLog(fragment, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Fragment shader compilation failed." << std::endl << infoLog;
        Util::endError();
        shaderSuccess = false;
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
        Util::beginError();
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Shader linking failed." << std::endl << infoLog;
        Util::endError();
        shaderSuccess = false;
    }

    // validate program
    glValidateProgram(id);
    glGetProgramiv(id, GL_VALIDATE_STATUS, &success);
    if (!success)
    {
        Util::beginError();
        glGetProgramInfoLog(id, 512, nullptr, infoLog);
        std::cout << "SHADER::LOAD_FROM_FILE::ERROR: Shader validation failed." << std::endl << infoLog;
        Util::endError();
        shaderSuccess = false;
    }

    // update m_ID
    m_ID = id;

    // no longer needed
    glDeleteShader(vertex);
    glDeleteShader(fragment);


    std::cout << "Loaded *" << m_shaderName << "* shader from files: `" << vertPath << "` `" << fragPath << "`\n";

    return shaderSuccess;
}

void Shader::use() const { glUseProgram(m_ID); }

unsigned int Shader::getShaderID() const { return m_ID; }


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

// ------ Shader manager ------
ShaderManager::ShaderManager(EngineObject* parent) : EngineObject{"ShaderManager", parent} {}

// load new shader
void ShaderManager::addShader(const std::string& name, const char* fragPath, const char* vertPath, Arena* arena)
{
    // create new shader and add it to arena
    Shader* shader{new Shader{name, this}};
    arena->addObject(shader);
    if (!shader->loadFromFile(fragPath, vertPath))
    {
        Util::beginError();
        std::cout << "SHADER_MANAGER::ADD_SHADER::ERROR: Failed to add shader `" << name << "`";
        Util::endError();
    }
    else
    {
        m_shaders.insert(std::pair{name, shader});
    }

    // load shader file
    getShader(name)->loadFromFile(fragPath, vertPath);
}

Shader* ShaderManager::getShader(const std::string& name) const
{
    if (shaderExists(name))
    {
        return m_shaders.find(name)->second;
    }
    Util::beginError();
    std::cout << "SHADER_MANAGER::GET_SHADER::ERROR: Shader `" << name << "` does not exist!";
    Util::endError();
    return nullptr;
}

void ShaderManager::useShader(const std::string& name) const
{
    if (shaderExists(name))
    {
        getShader(name)->use();
    }
    else
    {
        Util::beginError();
        std::cout << "SHADER_MANAGER::USE_SHADER::ERROR: Shader `" << name << "` does not exist!";
        Util::endError();
    }
}

bool ShaderManager::shaderExists(const std::string& name) const { return m_shaders.find(name) != m_shaders.end(); }
