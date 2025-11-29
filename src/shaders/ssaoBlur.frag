#version 410 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoTex;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTex, 0));
    float result = 0.0;
    
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            vec2 sampleCoord = TexCoords + offset;
      
            result += texture(ssaoTex, sampleCoord).r;
        } 
    }
    FragColor = result / 16.0;
}
