// Post processing shader that applies HDR and gamma correction.

#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

// sample from screen framebuffer
uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;
uniform sampler2D dirtMask;
uniform int useDirtMask = 0;

uniform float bloomStrength = 0.03f;
uniform float dirtMaskStrength = 20.0f;

uniform int useACES = 1;

// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

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
        dirt = texture(dirtMask, vec2(TexCoords.x, 1.0 - TexCoords.y)).rgb *
            dirtMaskStrength; // I feel like inverting the y
    }
    return mix(hdrColor, bloomColor + bloomColor * dirt, bloomStrength);
}

void main()
{
    vec3 hdrColor = bloom();

    vec3 mapped;
    if (useACES > 0)
    {
        // ACES tonemapping
        mapped = ACESFilm(hdrColor);
    }
    else
    {
        // Khronos PBR neutral note mapping
        mapped = PBRNeutralToneMapping(hdrColor);
    }

    // NOTE: We do gamma correction in the FXAA shader
    FragColor = vec4(mapped, 1.0);
}
