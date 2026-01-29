#version 410 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D tex;

void main()
{
    vec4 color = texture(tex, TexCoord);
    if (color.a < 0.01)
        discard;
    FragColor = color;
}
