#version 450
#extension GL_GOOGLE_include_directive : require

#include "include/tonemapper.glsl"

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return mix(vec3(0.0), vec3(1.0), (x*(a*x+b))/(x*(c*x+d)+e));
}

void main()
{
    vec4 color = texture(texSampler, fragUV);
    vec3 aces = ACESFilm(color.rgb);
    dither(aces);
    outColor = vec4(aces, color.a);
}