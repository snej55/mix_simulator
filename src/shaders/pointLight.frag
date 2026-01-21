#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
} fs_in;

uniform vec3 color;
uniform float radius;
uniform float intensity = 1.0;

void main()
{
    FragColor = vec4(color * radius * intensity, 1.0);
}
