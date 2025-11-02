#version 410 core

in vec2 TexCoord;
layout(location = 0) out vec3 downSample;

uniform sampler2D tex;
uniform vec2 srcResolution;

void main()
{
    const vec2 srcTexelSize = 1.0 / srcResolution;
    const float x = srcTexelSize.x;
    const float y = srcTexelSize.y;
    
    // take 13 samples around current texel
    const vec3 a = texture(tex, vec2(TexCoord.x - 2 * x, TexCoord.y + 2 * y)).rgb;
    const vec3 b = texture(tex, vec2(TexCoord.x, TexCoord.y + 2 * y)).rgb;
    const vec3 c = texture(tex, vec2(TexCoord.x + 2 * x, TexCoord.y + 2 * y)).rgb; 

    const vec3 d = texture(tex, vec2(TexCoord.x - 2 * x, TexCoord.y)).rgb;
    const vec3 e = texture(tex, vec2(TexCoord.x, TexCoord.y)).rgb;
    const vec3 f = texture(tex, vec2(TexCoord.x + 2 * x, TexCoord.y)).rgb; 

    const vec3 g = texture(tex, vec2(TexCoord.x - 2 * x, TexCoord.y - 2 * y)).rgb;
    const vec3 h = texture(tex, vec2(TexCoord.x, TexCoord.y - 2 * y)).rgb;
    const vec3 i = texture(tex, vec2(TexCoord.x + 2 * x, TexCoord.y - 2 * y)).rgb; 

    const vec3 j = texture(tex, vec2(TexCoord.x - x, TexCoord.y + y)).rgb;
    const vec3 k = texture(tex, vec2(TexCoord.x + x, TexCoord.y + y)).rgb;
    const vec3 l = texture(tex, vec2(TexCoord.x - x, TexCoord.y - y)).rgb; 
    const vec3 m = texture(tex, vec2(TexCoord.x + x, TexCoord.y - y)).rgb; 

    // apply weighted distrobution
    downSample = e * 0.125;
    downSample += (a + c + g + i) * 0.03125;
    downSample += (b + d + f + h) * 0.0625;
    downSample += (j + k + l + m) * 0.125;
}
