#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aTangent;
layout(location = 4) in ivec4 aBoneIds;
layout(location = 5) in vec4 aWeights;

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
    // for normal mapping
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
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
    vec4 totalPosition = vec4(0.0f);
    vec3 localNormal = aNormal;
    for (uint i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
	if (aBoneIds[i] == -1)
	    continue;
	if (aBoneIds[i] >= MAX_BONES)
	{
	    totalPosition = vec4(aPos, 1.0);
	    break;
	}
	
	vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0);
	totalPosition += localPosition * aWeights[i];
	localNormal = mat3(finalBonesMatrices[aBoneIds[i]]) * aNormal;
    }

    vs_out.FragPos = vec3(model * totalPosition);
    vs_out.TexCoords = aTexCoords;

    // create TBN matrix
    vec3 T = normalize(normalMat * aTangent.xyz);
    vec3 N = normalize(normalMat * aNormal);
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // get perpendicular vector B 
    // aTangent.w is tangent sign calculated using mikktspace.h to make sure tangent handedness is correct
    vec3 B = cross(N, T) * aTangent.w;
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;
    vs_out.TBN = TBN;

    gl_Position = projection * view * model * totalPosition;
}
