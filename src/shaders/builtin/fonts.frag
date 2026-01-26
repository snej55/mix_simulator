#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    // get alpha
    vec4 col = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    // apply alpha brightness to color
    FragColor = vec4(textColor, 1.0) * col;
}