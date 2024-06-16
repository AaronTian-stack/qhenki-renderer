#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    float seed;
    float intensity;
} pc;

void main()
{
    vec4 color = texture(texSampler, fragUV);
    float n = fract(sin(dot(fragUV, vec2(pc.seed + 12.9898, 78.233))) * 43758.5453);
    color.rgb *= (1.0 - pc.intensity + n * pc.intensity) * 1.1;
    outColor = color;
}