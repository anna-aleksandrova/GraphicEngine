#include "model.h"
#include "stb_image.h"
#include <iostream>
#include <algorithm>

// Global function to load a texture from a file
unsigned int TextureFromFile(const char *path, const std::string &directory) {
    std::string filename = std::string(path);

    // Remove potential Windows line-ending characters
    filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());
    filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());

    // Normalize path separators for Linux
    std::replace(filename.begin(), filename.end(), '\\', '/');

    // Extract just the filename if Assimp returned an absolute path
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }

    // Form the final correct path to the texture
    std::string fullPath = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    // Flip texture on load: OpenGL expects the Y-axis to start at 0 from the bottom
    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)      format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Configure texture wrapping and filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "[SUCCESS] Texture loaded successfully at: " << fullPath << std::endl;
    } else {
        std::cout << "[CRITICAL ERROR] Texture failed to load at path: " << fullPath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Model class constructor
Model::Model(std::string const &path) {
    loadModel(path);
}

// Render the model (calls Draw for each mesh)
void Model::Draw(GLuint shaderProgram) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shaderProgram);
    }
}

// Load the model using Assimp
void Model::loadModel(std::string const &path) {
    Assimp::Importer importer;
    // Process triangles, flip UV coordinates, and generate smooth normals
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    // Retrieve the directory path from the file path for texture loading
    directory = path.substr(0, path.find_last_of('/'));

    // Process the root node of the scene
    processNode(scene->mRootNode, scene);
}

// Recursively process nodes in the scene hierarchy
void Model::processNode(aiNode *node, const aiScene *scene) {
    // Process all meshes in the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    // Recursively process children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

// Translate aiMesh to our Mesh class
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;

        // Positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        // Normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Process indices for EBO
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Process material associated with the mesh
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Load diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // Load specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    return Mesh(vertices, indices, textures);
}

// Load textures from the material
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        // Check if texture was already loaded to avoid redundant memory usage
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // Cache the texture
        }
    }
    return textures;
}