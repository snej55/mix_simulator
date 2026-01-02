#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aTangent;
layout(location = 4) in ivec4 aBoneIDs;
layout(location = 5) in vec4 aWeights;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    // for normal mapping
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
    vec3 Normal;
    mat3 TBN;
}
vs_out;

// basic camera transformations
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

// for normal mapping
uniform vec3 lightPos;
uniform vec3 viewPos;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    // calculate bone influence
    vec4 totalPosition = vec4(0.0);
    vec3 localNormal = vec3(0.0);
    vec3 localTangent = vec3(0.0);
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        int boneID = aBoneIDs[i];
        float weight = aWeights[i];
        if (boneID < 0 || weight == 0.0 || boneID >= MAX_BONES)
        {
            continue;
        }

        vec4 localPosition = finalBonesMatrices[boneID] * vec4(aPos, 1.0);
        totalPosition += localPosition * weight;

        vec3 boneLocalNormal = mat3(finalBonesMatrices[boneID]) * aNormal;
        localNormal += boneLocalNormal * weight;
        vec3 boneLocalTangent = mat3(finalBonesMatrices[boneID]) * aTangent.xyz;
        localTangent += boneLocalTangent * weight;
    }

    if (totalPosition == vec4(0.0))
    {
        totalPosition = vec4(aPos, 1.0);
    }

    if (localNormal == vec3(0.0))
    {
        localNormal = aNormal;
    }

    if (localTangent == vec3(0.0))
    {
        localTangent = aTangent.xyz;
    }

    vs_out.FragPos = vec3(model * totalPosition);
    vs_out.TexCoords = aTexCoords;

    // create TBN matrix
    vec3 T = normalize(normalMat * localTangent);
    vec3 N = normalize(normalMat * localNormal);
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // get perpendicular vector B
    // aTangent.w is tangent sign calculated using mikktspace.h to make sure tangent handedness is correct
    vec3 B = cross(N, T) * aTangent.w;
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;
    vs_out.Normal = N;
    vs_out.TBN = TBN;

    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;
}
