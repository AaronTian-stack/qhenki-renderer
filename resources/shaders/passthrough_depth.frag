#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    vec4 clearColor;
} pc;

void main()
{
    vec4 tex = texture(texSampler, fragUV);
    vec3 color = tex.rgb;
    // alpha is used as true false flag
    if (pc.clearColor.a > 0.0 && tex.a == 0)
    {
        color = pc.clearColor.rgb;
    }
    outColor = vec4(color, 1.0);
}