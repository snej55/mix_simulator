#version 410 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// textures
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

const float PI = 3.14159265359;

// F0 = surface reflection at zero incidence
vec3 fresnelSchlick(float cosTheta, vec3 F0) { return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }

// normal distrobution function
float distroGGX(vec3 norm, vec3 h, float roughness)
{
    // looks better with roughness^2
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

void main()
{
    vec3 norm = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // outgoing radiance
    vec3 Lo = vec3(0.0);

    // ---- calculate light radiance ---- //

    vec3 L = normalize(lightPos - FragPos);
    // half-vector
    vec3 H = normalize(V + L);

    // distance to lightPos
    float dist = length(lightPos - FragPos);
    // standard realistic attenuation
    float attenuation = 1.0;
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BDRF
    // 1. Fresnel ratio
    // surface reflectance at zero incidence
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    // dot(H, V) = similarity with half-vector
    // calculate fresnel
    vec3 fresnel = fresnelSchlick(max(dot(H, V), 0.0), F0);
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

    // final color
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    FragColor = vec4(color, 1.0);
}
