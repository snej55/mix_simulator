#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    // get alpha
    float alpha = texture(text, TexCoords).r;
    // apply alpha brightness to color
    FragColor = vec4(textColor, alpha);
}
