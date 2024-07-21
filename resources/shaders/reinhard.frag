#version 450
#extension GL_GOOGLE_include_directive : require

#include "include/tonemapper.glsl"

void main()
{
    vec4 color = texture(texSampler, fragUV);
    color.rgb = color.rgb / (color.rgb + vec3(1.0));

    dither(color.rgb);

    outColor = color;
}