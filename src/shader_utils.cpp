#include "shader_utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "glad/glad.h"

// Reads the shader source code from a file and returns it as a string
std::string LoadShaderFromFile(const std::string &filePath) {
    std::ifstream shaderFile(filePath);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << shaderFile.rdbuf(); // Read the file content into the stream

    return buffer.str(); // Return the content as a std::string
}

// Compiles an individual shader and checks for compilation errors
GLuint createShader(std::string &filePath, GLuint shaderType) {
    auto vertexShaderCode_str = LoadShaderFromFile(filePath);
    auto vertexShaderCode = vertexShaderCode_str.c_str();

    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &vertexShaderCode, nullptr);
    glCompileShader(shader);

    // Check for shader compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << filePath << "\n" << infoLog << std::endl;
    }

    return shader;
}

// Links the vertex and fragment shaders into a single program and handles cleanup
GLuint createProgram(
    std::string &vertexShaderName,
    std::string &fragmentShaderName) {

    unsigned int vertexShader = createShader(vertexShaderName, GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader(fragmentShaderName, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error:\n" << infoLog << std::endl;
    }

    // Delete shaders as they are linked into the program and no longer needed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}