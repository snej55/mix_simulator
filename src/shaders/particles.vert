#version 410 core

layout(location = 0) in vec2 aQuad;
layout(location = 1) in vec3 aPos;
layout(location = 2) in vec3 aVel;
layout(location = 3) in float aSize;

out VS_OUT
{
    vec2 TexCoords;
    float size;
}
vs_out;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vs_out.TexCoords = aQuad + 0.5;
    vs_out.size = aSize;

    // strip rotation
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 WS = aPos + (right * aQuad.x * aSize) + (up * aQuad.y * aSize);

    gl_Position = projection * view * vec4(WS, 1.0);
}
