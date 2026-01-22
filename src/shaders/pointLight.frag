#version 410 core
out vec4 FragColor;

in VS_OUT { vec3 FragPos; }
fs_in;

uniform vec3 color;
uniform float radius;

void main() { FragColor = vec4(1.0, 0.0, 0.0, 1.0); }
