#version 450
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform Color {
    layout(offset = 64)
    vec4 color;
} color;

void main()
{
    outColor = vec4(color.color.rgb, 0.1);
}