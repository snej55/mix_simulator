#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

#define POINT_LIGHT_INTENSITY 1.0
struct Light
{
    vec3 position;
    vec3 color;
    float radius;
};

uniform sampler2D gPositionE;
uniform sampler2D gAlbedo;
uniform sampler2D gNormalE;
uniform sampler2D gARME;
uniform sampler2D gGeomNormal;
uniform int ssaoEnabled = 0;
uniform sampler2D ssao;
uniform float fogStrength = 1.0;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// CSM
uniform sampler2DArrayShadow shadowMap;
uniform float cascadePlaneDistances[16];
uniform mat4 lightSpaceMatrices[16];
uniform int cascadeCount;
uniform int csmEnabled = 0;
uniform float cascadeBlend = 0.5;

uniform vec3 viewPos;
uniform mat4 view;

const int MAX_POINT_LIGHTS = 32;
uniform int activeLights = 32;
uniform Light lights[MAX_POINT_LIGHTS];

const float PI = 3.14159265359;
const vec2 poissonDisk[16] =
    vec2[](vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725), vec2(-0.094184101, -0.92938870),
           vec2(0.34495938, 0.29387760), vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
           vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379), vec2(0.44323325, -0.97511554),
           vec2(0.53742981, -0.47373420), vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
           vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590), vec2(0.19984126, 0.78641367),
           vec2(0.14383161, -0.14100790));

// F0 = surface reflection at zero incidence
vec3 fresnelSchlick(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// normal distrobution function
float distroGGX(vec3 norm, vec3 h, float roughness)
{
    // looks better with roughness^2
    roughness = max(roughness, 0.0001);
    float a = roughness * roughness;
    float a2 = a * a; // a^2
    float NdotH = max(dot(norm, h), 0.0);
    float NdotH2 = NdotH * NdotH; // square it

    // numerator
    float num = a2;
    // denominator
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// geometry equation
float geomSchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    // numerator
    float num = NdotV;
    // denominator
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// the other geometry equation
float geomSmith(vec3 norm, vec3 view, vec3 light, float roughness)
{
    float NdotV = max(dot(norm, view), 0.0);
    float NdotL = max(dot(norm, light), 0.0);
    float ggx2 = geomSchlickGGX(NdotV, roughness);
    float ggx1 = geomSchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// https://github.com/vicrucann/shader-3dcurve/blob/master/src/Shaders/bezier.frag
float getFogFactor(float d)
{
    const float FogMax = 800.0;
    const float FogMin = 70.0;

    if (d >= FogMax)
        return 1.0;
    if (d <= FogMin)
        return 0.0;

    return 1 - (FogMax - d) / (FogMax - FogMin);
}

float angleHash(vec3 seed) { return fract(sin(dot(seed, vec3(12.9898, 78.233, 45.164))) * 43758.5453) * 2.0 * PI; }

float getCascadeShadow(int layer, vec3 fragPosWS, vec3 normal)
{
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    vec3 offsetPosWS = fragPosWS + normal * 0.005;
    vec4 fragPosLS = lightSpaceMatrices[layer] * vec4(offsetPosWS, 1.0);
    vec3 projCoords = fragPosLS.xyz / fragPosLS.w;
    projCoords = projCoords * 0.5 + 0.5;

    float currentDepth = projCoords.z;

    if (currentDepth > 1.0)
    {
        return 0.0;
    }

    float dzDx = dFdx(projCoords.z);
    float dzDy = dFdy(projCoords.z);
    vec2 duvDx = dFdx(projCoords.xy);
    vec2 duvDy = dFdy(projCoords.xy);

    float determinant = (duvDx.x * duvDy.y) - (duvDx.y * duvDy.x);
    float bias = 0.0001;
    if (abs(determinant) > 1e-6)
    {
        float inv = 1.0 / determinant;
        float dzDu = inv * (duvDy.y * dzDx - duvDx.y * dzDy);
        float dzDv = inv * (duvDx.x * dzDy - duvDy.x * dzDx);

        bias += abs(dzDu * texelSize.x) + abs(dzDv * texelSize.y);
    }
    bias = min(bias, 0.005);

    float angle = angleHash(fragPosWS);
    float s = sin(angle);
    float c = cos(angle);
    mat2 rotation = mat2(c, -s, s, c);

    const float spread = 0.0008;
    float shadow = 0.0;
    for (int i = 0; i < 16; ++i)
    {
        vec2 offset = rotation * poissonDisk[i];
        vec4 shadowCoords = vec4(projCoords.xy + offset * spread, float(layer), projCoords.z - bias);
        shadow += 1.0 - texture(shadowMap, shadowCoords);
    }

    shadow /= 16.0;

    return shadow * 0.8;
}

float getShadow(vec3 fragPosWS, vec3 normal)
{
    vec4 fragPosVS = view * vec4(fragPosWS, 1.0);
    float depth = -fragPosVS.z;

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depth < cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    if (layer == cascadeCount - 1 || cascadeBlend <= 0.0)
    {
        return getCascadeShadow(layer, fragPosWS, normal);
    }

    float split = cascadePlaneDistances[layer];
    float dist = max(0.000001, abs(depth - split));
    float fadeRange = 1.0;
    if (layer == cascadeCount - 2 &&
        abs(cascadePlaneDistances[layer] - cascadePlaneDistances[layer + 1]) >
            abs(cascadePlaneDistances[layer] - cascadePlaneDistances[layer - 1]))
    {
        fadeRange = 10.0;
    }
    float t = 1.0 - clamp(dist / fadeRange, 0.0, 1.0);

    if (t > 0.0)
    {
        float s0 = getCascadeShadow(layer, fragPosWS, normal);
        float s1 = getCascadeShadow(layer + 1, fragPosWS, normal);
        return mix(s0, s1, t);
    }
    return getCascadeShadow(layer, fragPosWS, normal);
}

vec3 octDecode(vec2 f)
{
    f = f * 2.0 - 1.0;
    vec3 n = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0.0, 1.0);
    n.x += (n.x >= 0.0) ? -t : t;
    n.y += (n.y >= 0.0) ? -t : t;
    return normalize(n);
}

void main()
{
    vec3 emissive;
    vec4 gPositionSample = texture(gPositionE, TexCoords);

    vec3 FragPosVS = gPositionSample.rgb;

    // convert to world space
    vec3 FragPos = vec3(inverse(view) * vec4(FragPosVS, 1.0));
    emissive.r = gPositionSample.a;

    vec4 albedoSample = texture(gAlbedo, TexCoords);
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2));
    float alpha = albedoSample.a;
    if (alpha < 0.1)
        discard;

    vec4 gNormalSample = texture(gNormalE, TexCoords);
    vec3 norm = normalize(gNormalSample.rgb);
    emissive.g = gNormalSample.a;

    vec4 gARMESample = texture(gARME, TexCoords);
    float ao = gARMESample.r;
    float roughness = gARMESample.g;
    float metallic = gARMESample.b;
    emissive.b = gARMESample.a;

    vec3 V = normalize(viewPos - FragPos);
    vec3 N = norm;
    float NdotV = max(dot(N, V), 0.0);

    // outgoing radiance
    vec3 Lo = vec3(0.0);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // ---- calculate light radiance ---- //

    for (int i = 0; i < activeLights; ++i)
    {
        vec3 diff = lights[i].position - FragPos;
        vec3 L = normalize(diff);
        // half-vector
        vec3 H = normalize(V + L);

        // standard realistic attenuation
        float dist2 = dot(diff, diff) / lights[i].radius / POINT_LIGHT_INTENSITY;
        float attenuation = 1.0 / dist2;
        vec3 radiance = lights[i].color * attenuation;

        // Cook-Torrance BDRF
        // 1. Fresnel ratio
        // surface reflectance at zero incidence
        // dot(H, V) = similarity with half-vector
        // calculate fresnel
        vec3 fresnel = fresnelSchlick(max(dot(H, V), 0.0), F0, roughness);
        // 2. Normal Distro-Function
        float NDF = distroGGX(norm, H, roughness);
        // 3. Geometry overshadowing function
        float geom = geomSmith(norm, V, L, roughness);

        // calculate BDRF
        vec3 num = NDF * geom * fresnel;
        float denom = 4.0 * max(dot(norm, V), 0.0) * max(dot(norm, L), 0.0) + 0.0001;
        vec3 specular = num / denom;

        // calculate specular contribution
        vec3 kS = fresnel; // specular
        vec3 kD = vec3(1.0) - kS; // diffuse
        kD *= 1.0 - metallic;

        // finally calculate outgoing radiance
        float NdotL = max(dot(norm, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // IBL
    vec3 FA = fresnelSchlick(NdotV, F0, roughness);
    vec3 kDA = vec3(1.0) - FA;
    kDA *= 1.0 - metallic;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 viewWS = normalize(viewPos - FragPos);
    vec3 R = reflect(-viewWS, norm);
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
    vec3 spec = prefilteredColor * (FA * brdf.x + brdf.y);

    vec3 irradiance = texture(irradianceMap, norm).rgb;
    vec3 diffuse = irradiance * albedo;

    // add SSAO
    float ssaoFactor = 1.0;
    if (ssaoEnabled > 0)
    {
        ssaoFactor = texture(ssao, TexCoords).r;
    }

    float shadow = 0.0;
    if (csmEnabled > 0)
    {
        vec2 enc = texture(gGeomNormal, TexCoords).rg;
        shadow = getShadow(FragPos, octDecode(enc));
    }

    vec3 ambient = (1.0 - shadow) * (diffuse * kDA + spec) * ssaoFactor;

    // get fog effect
    float fogAlpha = getFogFactor(abs(FragPosVS.z) * fogStrength);
    vec3 fogColor = textureLod(prefilterMap, normalize(FragPos - viewPos), MAX_REFLECTION_LOD).rgb;

    // final color
    vec3 color = ambient + emissive * fogAlpha + Lo;
    color = mix(color, fogColor, fogAlpha);

    FragColor = vec4(color, alpha);
}
