#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    float intensity;
} pc;

void main()
{
    float neighbor = pc.intensity * -1.0;
    float center = pc.intensity *  4.0 + 1.0;

    ivec2 texSize = textureSize(texSampler, 0);
    ivec2 fc = ivec2(gl_FragCoord.xy);

    vec3 color =
    texelFetchOffset(texSampler, fc, 0, ivec2(0, 1)).rgb * neighbor +
    texelFetchOffset(texSampler, fc, 0, ivec2(-1, 0)).rgb * neighbor +
    texelFetchOffset(texSampler, fc, 0, ivec2(0, 0)).rgb * center +
    texelFetchOffset(texSampler, fc, 0, ivec2(1, 0)).rgb * neighbor +
    texelFetchOffset(texSampler, fc, 0, ivec2(0, -1)).rgb * neighbor;

    outColor = vec4(color, 1.0);
}