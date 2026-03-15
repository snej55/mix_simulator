#version 410 core
out vec4 FragColor;

in VS_OUT
{
    vec2 TexCoords;
    float size;
} fs_in;

void main()
{
    // SDF lol
    float dist = distance(fs_in.TexCoords, vec2(0.5));
    if (dist > 0.5)
	discard;

    vec3 color = mix(vec3(1.0, 0.2, 0.1), vec3(1.0, 0.9, 0.4), fs_in.size);
    float alpha = (0.5 - dist) * 2.0 * fs_in.size;
    FragColor = vec4(color, alpha);
}
