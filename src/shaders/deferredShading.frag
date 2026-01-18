#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

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
uniform int ssaoEnabled = 0;
uniform sampler2D ssao;
uniform float fogStrength = 1.0;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 viewPos;
uniform mat4 view;

const int NUM_LIGHTS = 32;
uniform int activeLights = 32;
uniform Light lights[NUM_LIGHTS];

const float PI = 3.14159265359;

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
    const float FogMax = 500.0;
    const float FogMin = 50.0;

    if (d >= FogMax)
        return 1.0;
    if (d <= FogMin)
        return 0.0;

    return 1 - (FogMax - d) / (FogMax - FogMin);
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
        float dist2 = dot(diff, diff);
        float radius2 = lights[i].radius * lights[i].radius;
        float factor = clamp(1.0 - (dist2 * dist2) / (radius2 * radius2), 0.0, 1.0);
        float attenuation = factor * factor / max(dist2, 0.0001);
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

    vec3 ambient = (diffuse * kDA + spec) * ao * ssaoFactor;
    // final color
    vec3 color = emissive + ambient + Lo;

    // apply fog effect
    float fogAlpha = getFogFactor(abs(FragPosVS.z) * fogStrength);
    vec3 fogColor = textureLod(prefilterMap, normalize(FragPos - viewPos), MAX_REFLECTION_LOD).rgb;
    color = mix(color, fogColor, fogAlpha);
    // float fogAlpha = getFogFactor(-FragPosVS.z * 0.1);
    // vec3 fogColor = vec3(color.r * 0.2126 + color.g * 0.7152 + color.b + 0.0722) * 0.7; // weighted grayscale
    // color = mix(color, fogColor, fogAlpha);

    FragColor = vec4(color, alpha);
}
