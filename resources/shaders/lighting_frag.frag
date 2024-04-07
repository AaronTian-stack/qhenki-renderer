#version 450

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inputPosition;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inputAlbedo;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inputNormal;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inputRoughnessMetalAO;

layout(location = 0) in vec4 forward;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

void main()
{
    vec4 position = subpassLoad(inputPosition);
    vec4 albedo = subpassLoad(inputAlbedo);
    vec3 unconvertedNormal = subpassLoad(inputNormal).xyz;
    vec4 roughnessMetalAO = subpassLoad(inputRoughnessMetalAO);

    if (position.a == 0.0)
    {
        outColor = albedo;
        return;
    }

    // convert normal back to [-1, 1] range
    vec3 normal = unconvertedNormal * 2.0 - 1.0;
    float dot = clamp(dot(normalize(-forward.xyz), normalize(normal)), 0, 1);
    float diff = pow(dot * 0.5 + 0.5, 2.0);

    outColor = vec4(albedo.rgb * diff, 1.0);
}