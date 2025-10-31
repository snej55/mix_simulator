#version 410 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap;
uniform float roughness = -1.0;

void main()
{
    vec3 envColor;
    const float MAX_LOD = 4.0;
    if (roughness > 0.0)
    {
	envColor = textureLod(environmentMap, localPos, roughness * MAX_LOD).rgb;
    } else {
	envColor = texture(environmentMap, localPos).rgb;
    }
    FragColor = vec4(envColor, 1.0);
}
