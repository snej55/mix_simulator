// Post processing shader that applies HDR and gamma correction.

#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

// sample from screen framebuffer
uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform sampler2D dirtMask;
uniform int useDirtMask = 0;

uniform float bloomStrength = 0.04f;
uniform float dirtMaskStrength = 20.0f;

const float gamma = 2.2;

// https://github.com/KhronosGroup/ToneMapping/tree/main/PBR_Neutral
vec3 PBRNeutralToneMapping(vec3 color)
{
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;

    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;

    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression)
    return color;

    const float d = 1. - startCompression;
    float newPeak = 1. - d * d / (peak + d - startCompression);
    color *= newPeak / peak;

    float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
    return mix(color, newPeak * vec3(1, 1, 1), g);
}

vec3 bloom()
{
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    vec3 dirt = vec3(0.0);
    if (useDirtMask > 0)
    {
	vec3 dirt = texture(dirtMask, vec2(TexCoords.x, 1.0 - TexCoords.y)).rgb * dirtMaskStrength; // I feel like inverting the y
    }
    return mix(hdrColor, bloomColor + bloomColor * dirt, bloomStrength);
}

void main()
{
    vec3 hdrColor = bloom();

    // Khronos PBR neutral note mapping
    vec3 mapped = PBRNeutralToneMapping(hdrColor);

    // NOTE: We do gamma correction in the FXAA shader
    FragColor = vec4(mapped, 1.0);
}
