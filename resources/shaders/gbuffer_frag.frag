#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec4 outAlbedo; // location is index of framebuffer / attachment
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outColor;
layout(location = 3) out vec4 outMetalRoughnessAO;

// 80 is max number of samplers in any shader on macOS (with argument buffers turned on, validation still complains)
// since partial binding is on, indexing into undefined sampler will not show errors!
layout(set = 1, binding = 0) uniform sampler2D texSampler[60];

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

void main()
{
    outAlbedo = texture(texSampler[material.baseColorTexture], fragUV) * material.baseColorFactor;
    outNormal = vec4(normalize(normal), 0.0); // TODO: TBN matrix
    outColor = vec4(fragColor, 1.0);
    outMetalRoughnessAO = vec4(material.metallicFactor, material.roughnessFactor, 1.0, material.occlusionStrength);
}