#version 410 core
out vec2 FragColor;

uniform sampler2D gPositionE;
uniform sampler2D gNormalE;
uniform sampler2D texNoise;

#define SAMPLE_SIZE 24
#define NOISE_SIZE 4

uniform vec3 samples[SAMPLE_SIZE];

uniform int kernelSize = SAMPLE_SIZE;
uniform float radius = 0.5;
uniform float bias = 0.025;
uniform float rangeInfluence = 1.0;
uniform float power = 2.0;
uniform float theshold = 0.15;
uniform float minAO = 0.2;
uniform float scale = 100.0;

uniform int scrWidth;
uniform int scrHeight;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec2 TexCoords = gl_FragCoord.xy / vec2(float(scrWidth), float(scrHeight));

    // sample data from gBuffer is already in view space
    vec3 fragPosVS = texture(gPositionE, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormalE, TexCoords).rgb); // world space

    // use screen-space pixel coordinates for noise lookup
    vec2 noiseCoords = gl_FragCoord.xy / float(NOISE_SIZE);
    vec3 randomVec = normalize(texture(texNoise, noiseCoords).xyz);

    // convert normal to view space
    vec3 normalViewSpace = normalize(mat3(view) * normal);

    if (fragPosVS.z > -0.1)
    {
        FragColor = vec2(1.0, abs(fragPosVS.z / scale));
        return;
    }

    // create TBN matrix
    vec3 tangent = normalize(randomVec - normalViewSpace * dot(randomVec, normalViewSpace));
    vec3 bitangent = cross(normalViewSpace, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalViewSpace);

    float occlusion = 0.0;

    float validSamples = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = fragPosVS + TBN * samples[i] * radius;
        if (dot(samplePos, normalViewSpace) > 0.5)
            continue;

        vec3 rayDir = normalize(samplePos - fragPosVS);
        if (abs(dot(normalViewSpace, rayDir)) < theshold)
            continue;

        // project to screen space
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= max(offset.w, 1e-4);
        offset.xyz = offset.xyz * 0.5 + 0.5; // NDC to texture coordinates

        // discard samples outside screen
        if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
            continue;

        // already in view space
        vec3 samplePosVS = texture(gPositionE, offset.xy).xyz;

        // skip invalid samples
        if (length(samplePosVS) < 0.001)
            continue;

        float distanceToSample = length(samplePosVS - fragPosVS);

        float rangeCheck = smoothstep(radius * 1.2, radius * 0.2, distanceToSample);

        float depthDifference = samplePosVS.z - samplePos.z;
        float occlusionWeight = (depthDifference >= bias ? 1.0 : 0.0);
        occlusionWeight *= smoothstep(radius * rangeInfluence, 0.0, abs(depthDifference));

        occlusion += occlusionWeight * rangeCheck;
        validSamples += 1.0;
    }

    occlusion = 1.0 - (occlusion / float(max(validSamples, 1.0)));

    occlusion = clamp(occlusion, minAO, 1.0);

    FragColor = vec2(pow(occlusion, power), abs(fragPosVS.z / scale));
}
