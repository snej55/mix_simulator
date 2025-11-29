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
uniform float rangeInfluence = 1.0;
uniform float power = 4.0;

uniform int scrWidth;
uniform int scrHeight;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec2 noiseScale = vec2(float(scrWidth) / float(NOISE_SIZE), float(scrHeight) / float(NOISE_SIZE));

    // sample data from gBuffer
    vec3 fragPos = texture(gPositionE, TexCoords).xyz; // world space
    vec3 normal = normalize(texture(gNormalE, TexCoords).rgb); // world space
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // convert to view space
    vec3 fragPosVS = vec3(view * vec4(fragPos, 1.0));
    vec3 normalViewSpace = normalize(mat3(view) * normal);

    // create TBN matrix
    vec3 tangent = normalize(randomVec - normalViewSpace * dot(randomVec, normalViewSpace));
    vec3 bitangent = cross(normalViewSpace, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalViewSpace);

    float occlusion = 0.0;
    int validSamples = 0;
    
    // adaptive sampling: reduce kernel size for close-up geometry to maintain performance
    int effectiveKernelSize = kernelSize;
    float distance = abs(fragPosVS.z);
    if (distance < 5.0)  // very close, reduce samples significantly
        effectiveKernelSize = kernelSize / 8;
    else if (distance < 10.0)  // close, reduce samples moderately
        effectiveKernelSize = kernelSize / 4;
    
    for (int i = 0; i < kernelSize; ++i)
    {
        if (i >= effectiveKernelSize)
            break;
            
        // transform sample to view space
        vec3 samplePos = fragPosVS + TBN * samples[i] * radius;

        // project to screen space
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5; // NDC to texture coordinates

        // discard samples outside screen
        if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
            continue;

        // get depth at sample location (world space) and convert to view space
        vec3 sampleDepthWorldSpace = texture(gPositionE, offset.xy).xyz;
        
        // skip invalid samples
        if (length(sampleDepthWorldSpace) < 0.001)
            continue;
        
        float sampleDepth = (view * vec4(sampleDepthWorldSpace, 1.0)).z;

        // range check - fade occlusion based on distance
        float rangeCheck = smoothstep(0.0, 1.0, radius / (abs(fragPosVS.z - sampleDepth) * rangeInfluence));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
        validSamples++;
    }
    
    if (validSamples == 0)
    {
        FragColor = 1.0;
        return;
    }
    
    occlusion = 1.0 - (occlusion / float(validSamples));
    FragColor = pow(occlusion, power);
}
