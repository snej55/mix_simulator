#version 410 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D tex;
uniform vec3 tint = vec3(1.0);

void main()
{
    vec4 color = texture(tex, TexCoord);
    color.rgb = color.rgb * tint;
    if (color.a < 0.01)
        discard;
    FragColor = color;
}
