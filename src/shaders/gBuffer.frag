#version 410 core
layout(location = 0) out vec4 gPositionE; // xyz = Position, w = Emissive.r
layout(location = 1) out vec4 gAlbedo; // xyzw = Albedo.rgba
layout(location = 2) out vec4 gNormalE; // xyz = Normal (world space), w = Emissive.g
layout(location = 3) out vec4 gARME; // x = AO, y = Roughness, z = Metallic, w = Emissive.b

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    // for normal mapping
    vec3 Tangent;
    float BitangentSign;
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

uniform mat4 view;

void main()
{
    vec4 albedoSample = (material.useAlbedoTex == 1) ? texture(material.albedoMap, fs_in.TexCoords) : material.albedo;
    float metallic =
        (material.useMetallicTex == 1) ? texture(material.metallicMap, fs_in.TexCoords).r : material.metallic;
    float roughness =
        (material.useRoughnessTex == 1) ? texture(material.roughnessMap, fs_in.TexCoords).r : material.roughness;
    float ao = (material.useAOTex == 1) ? texture(material.aoMap, fs_in.TexCoords).r : 1.0;
    vec3 emissive = (material.useEmissiveTex == 1) ? texture(material.emissiveMap, fs_in.TexCoords).rgb
                                                   : material.emissiveFactor * material.emissiveIntensity;

    // calculate world space normal
    vec3 normal;
    if (material.useNormalTex == 1)
    {
        // reorthogonalize TBN to fix handedness after fragment interpolation
        vec3 N = normalize(fs_in.Normal);
        vec3 T = normalize(fs_in.Tangent - N * dot(N, fs_in.Tangent));
        vec3 B = cross(N, T) * fs_in.BitangentSign;
        mat3 TBN = mat3(T, B, N);

        // sample normal map (tangent space)
        normal = texture(material.normalMap, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(TBN * normal);
    }
    else
    {
        normal = normalize(fs_in.Normal);
    }

    // store view-space pos instead of world-space for better precision with GL_RGBA16F (reduce memory bandwidth too)
    vec3 FragPosVS = vec3(view * vec4(fs_in.FragPos, 1.0));
    gPositionE = vec4(FragPosVS, emissive.r);
    gAlbedo = albedoSample;
    gNormalE = vec4(normal, emissive.g);
    // we don't really need ao btw :)
    gARME = vec4(ao, roughness, metallic, emissive.b);
}
