#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D tex;

void main()
{
    // get alpha
    float alpha = texture(tex, TexCoords).r;
    FragColor = vec4(vec3(1.0), alpha);
}
