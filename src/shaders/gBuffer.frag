#version 410 core
layout (location = 0) out vec4 gPositionE; // xyz = Position, w = Emissive.r
layout (location = 1) out vec4 gAlbedo; // xyzw = Albedo.rgba
layout (location = 2) out vec4 gNormalE; // xyz = Normal (world space), w = Emissive.g
layout (location = 3) out vec4 gARME; // x = AO, y = Roughness, z = Metallic, w = Emissive.b

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    // for normal mapping
    mat3 TBN;
}
fs_in;

struct Material
{
    int useAlbedoTex;
    int useMetallicTex;
    int useRoughnessTex;
    int useNormalTex;
    int useAOTex;
    int useEmissiveTex;

    sampler2D albedoMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;
    sampler2D normalMap;
    sampler2D emissiveMap;

    vec4 albedo;
    float metallic;
    float roughness;
    vec3 emissiveFactor;
    float emissiveIntensity;
};

uniform Material material;

void main()
{
    vec4 albedoSample = (material.useAlbedoTex == 1) ? texture(material.albedoMap, fs_in.TexCoords) : material.albedo;
    float metallic = (material.useMetallicTex == 1) ? texture(material.metallicMap, fs_in.TexCoords).r : material.metallic;
    float roughness = (material.useRoughnessTex == 1) ? texture(material.roughnessMap, fs_in.TexCoords).r : material.roughness;
    float ao = (material.useAOTex == 1) ? texture(material.aoMap, fs_in.TexCoords).r : 1.0;
    vec3 emissive = (material.useEmissiveTex == 1) ? texture(material.emissiveMap, fs_in.TexCoords).rgb : material.emissiveFactor * material.emissiveIntensity;

    // calculate world space normal
    vec3 normal;
    if (material.useNormalTex == 1)
    {
        normal = texture(material.normalMap, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0); // normal in tangent space
        normal = normalize(fs_in.TBN * normal);
    } else {
        normal = normalize(fs_in.Normal);
    }

    gPositionE = vec4(fs_in.FragPos, emissive.r);
    gAlbedo = albedoSample;
    gNormalE = vec4(normal, emissive.g);
    gARME = vec4(ao, roughness, metallic, emissive.b);
}
