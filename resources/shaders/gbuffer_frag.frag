#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outAlbedo; // location is index of framebuffer / attachment
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMetalRoughnessAO;

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

void setValues(out vec3 albedo, out vec3 normal, out vec2 metalRoughness, out float AO, out vec3 emissive)
{
    if (material.baseColorTexture != -1)
        albedo = texture(texSampler[material.baseColorTexture], fragUV).rgb;
    else
        albedo = material.baseColorFactor.rgb;

    if (material.normalTexture != -1)
    {
        // TODO: TBN calculation
        vec3 N = texture(texSampler[material.normalTexture], fragUV).rgb;
        normal = normalize(N * 2.0 - 1.0);
    }
    else
        normal = normalize(fragNormal);

    if (material.metallicRoughnessTexture != -1)
        metalRoughness = texture(texSampler[material.metallicRoughnessTexture], fragUV).rg;
    else
        metalRoughness = vec2(material.metallicFactor, material.roughnessFactor);

    if (material.occlusionTexture != -1)
        AO = texture(texSampler[material.occlusionTexture], fragUV).r;
    else
        AO = material.occlusionStrength;

    if (material.emissiveTexture != -1)
        emissive = texture(texSampler[material.emissiveTexture], fragUV).rgb;
    else
        emissive = vec3(0.0);
}

void main()
{
    vec3 albedo;
    vec3 normal;
    vec2 metalRoughness;
    float AO;
    vec3 emissive;
    setValues(albedo, normal, metalRoughness, AO, emissive);

    outAlbedo = vec4(albedo, 1.0);
    outNormal = vec4(normalize(normal), 1.0);
    outMetalRoughnessAO = vec4(metalRoughness, AO, 1.0);
}