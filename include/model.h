#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"

#include <string>
#include <vector>
#include <iostream>

// Helper function to load textures from file
unsigned int TextureFromFile(const char *path, const std::string &directory);

class Model {
public:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded; // Texture cache to prevent loading the same texture twice

    // Constructor expects the file path to the 3D model
    Model(std::string const &path);

    // Draws the entire model (all its meshes)
    void Draw(GLuint shaderProgram);

private:
    void loadModel(std::string const &path);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};
