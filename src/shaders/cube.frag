#version 410 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;

uniform vec3 albedo;

// basic phong lighting
void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(viewPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    FragColor = vec4(albedo * diff, 1.0);
}
