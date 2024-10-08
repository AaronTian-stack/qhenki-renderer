#version 450

layout(set = 1, binding = 0) uniform samplerCube cubemap;

layout(location = 0) in vec3 localPos;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

void main()
{
    vec3 envColor = texture(cubemap, localPos).rgb;
    outColor = vec4(envColor, 1.0);
}