#version 410 core

in vec2 TexCoord;
layout(location = 0) out vec3 upSample;

uniform sampler2D tex;
uniform float filterRadius;

void main()
{
    float x = filterRadius;
    float y = filterRadius;

    // take 9 samples
    vec3 a = texture(tex, vec2(TexCoord.x - x, TexCoord.y + y)).rgb;
    vec3 b = texture(tex, vec2(TexCoord.x, TexCoord.y + y)).rgb;
    vec3 c = texture(tex, vec2(TexCoord.x + x, TexCoord.y + y)).rgb;

    vec3 d = texture(tex, vec2(TexCoord.x - x, TexCoord.y)).rgb;
    vec3 e = texture(tex, vec2(TexCoord.x, TexCoord.y)).rgb;
    vec3 f = texture(tex, vec2(TexCoord.x + x, TexCoord.y)).rgb;

    vec3 g = texture(tex, vec2(TexCoord.x - x, TexCoord.y - y)).rgb;
    vec3 h = texture(tex, vec2(TexCoord.x, TexCoord.y - y)).rgb;
    vec3 i = texture(tex, vec2(TexCoord.x + x, TexCoord.y - y)).rgb;

    // apply weighted distrobution
    upSample = e * 4.0;
    upSample += (b + d + f + h) * 2.0;
    upSample += (a + c + g + i);
    upSample *= 0.0625;
}
