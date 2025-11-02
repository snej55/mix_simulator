#version 410 core

in vec2 TexCoord;
layout(location = 0) out vec3 downSample;

uniform sampler2D tex;
uniform vec2 srcResolution;
uniform int mipLevel;

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v) { return PowVec3(v, invGamma); }

float RGBToLuminance(vec3 col)
{
    return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
}

float KarisAverage(vec3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = RGBToLuminance(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}

void main()
{
    vec2 srcTexelSize = 1.0 / srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;
    
    // take 13 samples around current texel
    vec3 a = texture(tex, vec2(TexCoord.x - 2 * x, TexCoord.y + 2 * y)).rgb;
    vec3 b = texture(tex, vec2(TexCoord.x, TexCoord.y + 2 * y)).rgb;
    vec3 c = texture(tex, vec2(TexCoord.x + 2 * x, TexCoord.y + 2 * y)).rgb; 

    vec3 d = texture(tex, vec2(TexCoord.x - 2 * x, TexCoord.y)).rgb;
    vec3 e = texture(tex, vec2(TexCoord.x, TexCoord.y)).rgb;
    vec3 f = texture(tex, vec2(TexCoord.x + 2 * x, TexCoord.y)).rgb; 

    vec3 g = texture(tex, vec2(TexCoord.x - 2 * x, TexCoord.y - 2 * y)).rgb;
    vec3 h = texture(tex, vec2(TexCoord.x, TexCoord.y - 2 * y)).rgb;
    vec3 i = texture(tex, vec2(TexCoord.x + 2 * x, TexCoord.y - 2 * y)).rgb; 

    vec3 j = texture(tex, vec2(TexCoord.x - x, TexCoord.y + y)).rgb;
    vec3 k = texture(tex, vec2(TexCoord.x + x, TexCoord.y + y)).rgb;
    vec3 l = texture(tex, vec2(TexCoord.x - x, TexCoord.y - y)).rgb; 
    vec3 m = texture(tex, vec2(TexCoord.x + x, TexCoord.y - y)).rgb; 

    // apply weighted distrobution
    vec3 groups[5];
    switch (mipLevel)
    {
	case 0:
	    groups[0] = (a+b+d+e) * (0.125f/4.0f);
	    groups[1] = (b+c+e+f) * (0.125f/4.0f);
	    groups[2] = (d+e+g+h) * (0.125f/4.0f);
	    groups[3] = (e+f+h+i) * (0.125f/4.0f);
	    groups[4] = (j+k+l+m) * (0.5f/4.0f);
	    groups[0] *= KarisAverage(groups[0]);
	    groups[1] *= KarisAverage(groups[1]);
	    groups[2] *= KarisAverage(groups[2]);
	    groups[3] *= KarisAverage(groups[3]);
	    groups[4] *= KarisAverage(groups[4]);
	    downSample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
	    break;
	default:
	    downSample = e * 0.125;
	    downSample += (a + c + g + i) * 0.03125;
	    downSample += (b + d + f + h) * 0.0625;
	    downSample += (j + k + l + m) * 0.125;
	    break;
    }
}
