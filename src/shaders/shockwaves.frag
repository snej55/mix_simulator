// Created by Jens Kromdijk 17/01/2026
// Shockwaves code taken from https://www.geeks3d.com/20091116/shader-library-2d-shockwave-post-processing-filter-glsl/

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

uniform vec2 center;
uniform float time;
uniform float scrWidth;
uniform float scrHeight;
uniform vec3 shockParams = vec3(10.0, 0.8, 0.1);

uniform float stitchingSize = 12.0;
uniform int invert = 1;

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

vec3 bloom(vec2 uv)
{
    vec3 hdrColor = texture(screenTexture, uv).rgb;
    vec3 bloomColor = texture(bloomBlur, uv).rgb;
    vec3 dirt = vec3(0.0);
    if (useDirtMask > 0)
    {
        dirt = texture(dirtMask, vec2(uv.x, 1.0 - uv.y)).rgb * dirtMaskStrength; // I feel like inverting the y
    }
    return mix(hdrColor, bloomColor + bloomColor * dirt, bloomStrength);
}

vec3 crossStitch(vec2 uv)
{
    vec3 c = vec3(0.0);
    float size = stitchingSize;
    vec2 cPos = uv * vec2(scrWidth, scrHeight);
    vec2 tlPos = floor(cPos / vec2(size, size));
    tlPos *= size;
    int remX = int(mod(cPos.x, size));
    int remY = int(mod(cPos.y, size));
    if (remX == 0 && remY == 0)
    {
        tlPos = cPos;
    }
    vec2 blPos = tlPos;
    blPos.y += (size - 1.0);
    if ((remX == remY) || (((int(cPos.x) - int(blPos.x)) == (int(blPos.y) - int(cPos.y)))))
    {
        if (invert == 1)
        {
            c = vec3(0.0, 0.0, 0.0);
        }
        else
        {
            c = bloom(tlPos * vec2(1.0 / scrWidth, 1.0 / scrHeight)) * 1.4;
        }
    }
    else
    {
        if (invert == 1)
        {
            c = bloom(tlPos * vec2(1.0 / scrWidth, 1.0 / scrHeight)) * 1.4;
        }
        else
        {
            c = vec3(0.0, 0.0, 0.0);
        }
    }

    return c;
}

void main()
{
    vec2 uv = TexCoords;
    vec2 texCoord = uv;
    float aspectR = scrWidth / scrHeight;

    vec2 corrUV = vec2(uv.x * aspectR, uv.y);
    vec2 corrCenter = vec2(center.x * aspectR, center.y);
    float dist = distance(corrUV, corrCenter);
    if ((dist <= (time + shockParams.z)) && (dist >= (time - shockParams.z)))
    {
        float diff = (dist - time);
        float powDiff = 1.0 - pow(abs(diff * shockParams.x), shockParams.y);
        float diffTime = diff * powDiff;
        vec2 diffUV = normalize(corrUV - corrCenter);

        texCoord.x = uv.x + (diffUV.x * diffTime) / aspectR;
        texCoord.y = uv.y + (diffUV.y * diffTime);
    }
    vec3 hdrColor = bloom(texCoord);

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
