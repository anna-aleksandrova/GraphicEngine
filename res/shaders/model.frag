#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Texture loaded from C++
uniform sampler2D texture_diffuse1;

// Material structure defined by the assignment requirements
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

uniform vec3 viewPos;
uniform Material material;
uniform PointLight light;

void main() {
    // 1. Fetch texture color
    vec3 texColor = vec3(texture(texture_diffuse1, TexCoords));

    // 2. Lighting geometry calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    // 3. Ambient lighting
    // Combining the light's ambient component, the material's ambient vector, and the texture
    vec3 ambient = light.ambient * material.ambient * texColor;

    // 4. Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    // Combining the light's diffuse component, the material's diffuse vector, and the texture
    vec3 diffuse = light.diffuse * (diff * material.diffuse * texColor);

    // 5. Specular lighting (Blinn-Phong model)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    // Specular highlights use the material's specular vector
    vec3 specular = light.specular * (spec * material.specular);

    // 6. Light attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 7. Final output compilation
    vec3 result = (ambient + diffuse + specular) * attenuation;
    FragColor = vec4(result, 1.0);
}
