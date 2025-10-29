#version 410 core

out vec4 FragColor;

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    mat3 TBN;
}
fs_in;

uniform vec3 lightColor;

// textures
uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D normalMap;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 viewPos;
uniform vec3 lightPos;

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

void main()
{
    // albedo with g.c
    vec3 albedo = pow(texture(albedoMap, fs_in.TexCoords).rgb, vec3(2.2));
    float metallic = texture(metallicMap, fs_in.TexCoords).r;
    float roughness = texture(roughnessMap, fs_in.TexCoords).r;
    float ao = texture(aoMap, fs_in.TexCoords).r;

    vec3 norm = texture(normalMap, fs_in.TexCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0); // normal in tangent space
    // norm = Normal;
    vec3 V = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);

    // outgoing radiance
    vec3 Lo = vec3(0.0);

    // ---- calculate light radiance ---- //

    vec3 L = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    // half-vector
    vec3 H = normalize(V + L);

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

    // IBL
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 R = reflect(-V, norm);
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(norm, V), 0.0), roughness)).rg;
    vec3 spec = prefilteredColor * (fresnel * brdf.x + brdf.y);

    vec3 normWS = normalize(transpose(fs_in.TBN) * norm); // world space normal
    vec3 irradiance = texture(prefilterMap, normWS).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (diffuse * kD + spec) * ao;
    // final color
    vec3 color = ambient + Lo;

    FragColor = vec4(spec, 1.0);
}
