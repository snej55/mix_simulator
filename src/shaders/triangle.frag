#version 410 core
out vec4 FragColor;

in vec3 Pos;

void main() { FragColor = vec4(1.0 - Pos.x, 1.0 - Pos.y, 1.0 - Pos.x - Pos.y, 1.0); }
