#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

void main()
{
    vec3 color = texture(texSampler, fragUV).rgb;

    outColor = vec4(color, 1.0);
}