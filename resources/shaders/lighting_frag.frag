#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputAlbedo;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputRoughnessMetalAO;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

void main()
{
    vec4 albedo = subpassLoad(inputAlbedo);

    outColor = albedo;
}