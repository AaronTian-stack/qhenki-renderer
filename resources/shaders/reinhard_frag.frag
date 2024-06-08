#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

void main()
{
    vec4 color = texture(texSampler, fragUV);
    color.rgb = color.rgb / (color.rgb + vec3(1.0));
    outColor = color;
}