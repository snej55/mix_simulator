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

uniform int scrWidth;
uniform int scrHeight;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vec2 TexCoords = gl_FragCoord.xy / vec2(float(scrWidth), float(scrHeight));

    // sample data from gBuffer
    vec3 fragPos = texture(gPositionE, TexCoords).xyz; // world space
    vec3 normal = normalize(texture(gNormalE, TexCoords).rgb); // world space
    
    // use screen-space pixel coordinates for noise lookup
    vec2 noiseCoords = gl_FragCoord.xy / float(NOISE_SIZE);
    vec3 randomVec = normalize(texture(texNoise, noiseCoords).xyz);

    // convert to view space
    vec3 fragPosVS = vec3(view * vec4(fragPos, 1.0));
    vec3 normalViewSpace = normalize(mat3(view) * normal);

    // create TBN matrix
    vec3 tangent = normalize(randomVec - normalViewSpace * dot(randomVec, normalViewSpace));
    vec3 bitangent = cross(normalViewSpace, tangent);
    mat3 TBN = mat3(tangent, bitangent, normalViewSpace);

    vec3 viewRow2 = vec3(view[0][2], view[1][2], view[2][2]);
    float viewZ = view[3][2];

    float occlusion = 0.0;
    
    float validSamples = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        // transform sample to view space
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

        // get depth at sample location (world space) and convert to view space
        vec3 sampleDepthWorldSpace = texture(gPositionE, offset.xy).xyz;
        
        // skip invalid samples
        if (length(sampleDepthWorldSpace) < 0.001)
            continue;
        
        float sampleDepth = dot(viewRow2, sampleDepthWorldSpace) + viewZ;

        // range check - fade occlusion based on distance
        float rangeCheck = smoothstep(0.0, 1.0, radius / (abs(fragPosVS.z - sampleDepth) * rangeInfluence));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
        validSamples += 1.0;
    }
    
    occlusion = 1.0 - (occlusion / float(max(validSamples, 1.0)));
    FragColor = vec2(pow(occlusion, power), abs(fragPosVS.z / 100.0));
}
