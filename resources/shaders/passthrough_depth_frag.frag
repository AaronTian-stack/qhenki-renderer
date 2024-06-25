#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 0, binding = 1) uniform sampler2D depth;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    vec4 clearColor;
} pc;

void main()
{
    vec3 color = texture(texSampler, fragUV).rgb;
    float depth = texture(depth, fragUV).r;
    if (pc.clearColor.a > 0.0 && depth == 1.0)
    {
        color = pc.clearColor.rgb;
    }
    outColor = vec4(color, 1.0);
}