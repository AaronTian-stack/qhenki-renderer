#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

void main()
{
    vec3 color = texture(texSampler, fragUV).rgb;
    color = color / (color + vec3(1.0));
    outColor = vec4(color, 1.0);
}