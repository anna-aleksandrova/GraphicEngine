#ifndef OPENGL_START_SHADER_UTILS_H
#define OPENGL_START_SHADER_UTILS_H
#include <string>

#include "glad/glad.h"


std::string LoadShaderFromFile(const std::string &filePath);
GLuint  createShader(std::string &filePath, GLuint shaderType);
GLuint createProgram(
    std::string &vertexShaderName,
    std::string &fragmentShaderName);

#endif //OPENGL_START_SHADER_UTILS_H