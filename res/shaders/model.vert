#version 330 core

// Input vertex attributes from VBO
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Outputs to the fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Transformation matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculate the position of the fragment in world space
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transform normals to world space using the Normal Matrix
    // We use mat3(transpose(inverse(model))) to prevent normal distortion during non-uniform scaling
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass texture coordinates directly to the fragment shader
    TexCoords = aTexCoords;

    // Calculate the final position of the vertex on the screen
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
