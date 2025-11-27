#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionE;
uniform sampler2D gNormalE;
uniform sampler2D texNoise;

#define SAMPLE_SIZE 64
#define NOISE_SIZE 4

uniform vec3 samples[SAMPLE_SIZE];

uniform int kernelSize = SAMPLE_SIZE;
uniform float radius = 0.5;
uniform float bias = 0.025;

uniform int scrWidth;
uniform int scrHeight;

uniform mat4 projection;

void main()
{
    vec2 noiseScale = vec2(float(scrWidth) / float(NOISE_SIZE), float(scrHeight) / float(NOISE_SIZE));

    // sample data from gBuffer
    vec3 fragPos = texture(gPositionE, TexCoords).xyz;
    vec3 normal = texture(gNormalE, TexCoords).rgb;
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

    // create TBN matrix
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
	vec3 samplePos = TBN * samples[i];
	samplePos = fragPos + samplePos * radius;

	vec4 offset = vec4(samplePos, 1.0);
	offset = projection * offset;
	offset.xyz /= offset.w;
	offset.xyz = offset.xyz * 0.5 + 0.5; // transform from 0.0 to 1.0

	float sampleDepth = texture(gPositionE, offset.xy).z;

	float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
	occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);

    FragColor = occlusion;
}
