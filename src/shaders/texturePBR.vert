#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aTangent;

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

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
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

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
