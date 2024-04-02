#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 lightV;

layout(location = 0) out vec4 outColor; // index of framebuffer / attachment

// 16 is max number of samplers in any shader on macOS
// since partial binding is on, indexing into undefined sampler will not show errors!
layout(set = 1, binding = 0) uniform sampler2D texSampler[16];

layout(push_constant) uniform mats {
    layout(offset = 64) vec4 baseColorFactor;
    int baseColorTexture;

    float metallicFactor;
    float roughnessFactor;
    int metallicRoughnessTexture;

    int normalTexture;

    int occlusionTexture;
    float occlusionStrength;

    int emissiveTexture;
} material;


// TODO: uniform buffers for lights

void main()
{
    // lambert
    float dot = dot(normalize(lightV), normalize(normal));
    // half lambert

    float diff = pow(dot * 0.5 + 0.5, 2.0);

    vec3 normalV = normal + vec3(1.0) * 0.5;

    vec4 baseColor = texture(texSampler[material.baseColorTexture], fragUV);
    // undo gamma correction for base color since it was loaded as a unorm
    baseColor.rgb = pow(baseColor.rgb, vec3(2.2));
    baseColor *= material.baseColorFactor;

    vec3 c = baseColor.rgb * diff;

    vec3 emissive = texture(texSampler[material.emissiveTexture], fragUV).rgb;
    //c += emissive;

    vec3 mr = texture(texSampler[material.metallicRoughnessTexture], fragUV).rgb;
    float metallic = mr.r;
    float roughness = mr.g;

    outColor = vec4(c, 1.0);
}