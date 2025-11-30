#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoTex;

#define NOISE_SIZE 4

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));
    
    float result = 0.0;
    vec2 hlim = vec2(float(-NOISE_SIZE) * 0.5 + 0.5);
    for (int x = 0; x < NOISE_SIZE; ++x)
    {
        for (int y = 0; y < NOISE_SIZE; ++y)
        {
            vec2 offset = (hlim + vec2(float(x), float(y))) * texelSize;
            result += texture(ssaoTex, TexCoords + offset).r;
        }
    }
    FragColor = result / float(NOISE_SIZE * NOISE_SIZE);
}

// bilateral blur version
// #version 410 core
// out float FragColor;

// in vec2 TexCoords;

// uniform sampler2D ssaoTex;

// uniform float blurRadius = 3.0;        // blur radius in pixels
// uniform float spatialSigma = 2.0; // spatial gaussian sigma
// uniform float depthSigma = 1.5;   // range gaussian sigma

// void main()
// {
//     vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));

//     // center values
//     vec4 centerSample = texture(ssaoTex, TexCoords);
//     float centerAO = centerSample.r;
//     float centerDepth = centerSample.g;

//     int radius = int(max(1.0, blurRadius));
//     float twoSpatialSigma2 = 2.0 * spatialSigma * spatialSigma;
//     float twoDepthSigma2 = 2.0 * depthSigma * depthSigma;

//     float result = 0.0;
//     float totalWeight = 0.0;

//     for (int x = -radius; x <= radius; ++x)
//     {
//         for (int y = -radius; y <= radius; ++y)
//         {
//             vec2 offset = vec2(float(x), float(y)) * texelSize;
//             vec2 coord = clamp(TexCoords + offset, texelSize, 1.0 - texelSize);

//             vec4 texSample = texture(ssaoTex, coord);
//             float sampleAO = texSample.r;
//             float sampleDepth = texSample.g;

//             float spatialDist2 = float(x * x + y * y);
//             float spatialWeight = exp(-spatialDist2 / twoSpatialSigma2);

//             float depthDiff = sampleDepth - centerDepth;
//             float depthWeight = exp(-(depthDiff * depthDiff) / twoDepthSigma2);

//             float weight = spatialWeight * depthWeight;
//             result += sampleAO * weight;
//             totalWeight += weight;
//         }
//     }

//     if (totalWeight > 0.0)
//     {
//         result /= totalWeight;
//     } else {
//         result = centerAO;
//     }
//     FragColor = result;
// }