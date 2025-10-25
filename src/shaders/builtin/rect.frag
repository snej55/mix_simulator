#version 410 core
out vec4 FragColor;

uniform vec3 shapeColor;

void main()
{
    FragColor = vec4(shapeColor, 1.0);
}