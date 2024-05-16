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
    vec2 uv = fragUV * (1.0 - fragUV.yx);
    float vig = uv.x * uv.y * 15.0;
    vig = pow(vig, pc.intensity);
    outColor = vec4(vig);
}