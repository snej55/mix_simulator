#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionE;
uniform sampler2D gAlbedo;
uniform sampler2D gNormalE;
uniform sampler2D gARME;

uniform vec3 lightColor;

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
    vec3 emissive;
    vec4 gPositionSample = texture(gPositionE, TexCoords);
    vec3 FragPos = gPositionSample.rgb;
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

    // outgoing radiance
    vec3 Lo = vec3(0.0);

    // ---- calculate light radiance ---- //

    vec3 L = normalize(lightPos - FragPos);
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
    vec3 viewWS = normalize(viewPos - FragPos);
    vec3 R = reflect(-viewWS, norm);
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(norm, viewWS), 0.0), roughness)).rg;
    vec3 spec = prefilteredColor * (fresnel * brdf.x + brdf.y);

    vec3 irradiance = texture(irradianceMap, norm).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient = (diffuse * kD + spec) * ao;
    // final color
    vec3 color = emissive + ambient + Lo;

    FragColor = vec4(color, alpha);
}
