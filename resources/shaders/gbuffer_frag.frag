#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragTangent;
layout(location = 5) in vec3 fragBiTangent;

layout(location = 0) out vec4 outAlbedo; // location is index of framebuffer / attachment
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMetalRoughnessAO;
layout(location = 3) out vec4 outEmissive;

// 80 is max number of samplers in any shader on macOS (with argument buffers turned on, validation still complains)
// since partial binding is on, indexing into undefined sampler will not show errors!
layout(set = 1, binding = 0) uniform sampler2D texSampler[80];

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

const float[16] dither =
{
1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
};

void setValues(out vec4 albedo, out vec3 normal, out vec4 metalRoughness, out float AO, out vec3 emissive)
{
    if (material.baseColorTexture != -1)
    {
        albedo = texture(texSampler[material.baseColorTexture], fragUV);
        albedo.rgb = pow(albedo.rgb, vec3(2.2));
    }
    else
        albedo = material.baseColorFactor;

    if (material.normalTexture != -1)
    {
        mat3 TBN = mat3(normalize(fragTangent), normalize(fragBiTangent), normalize(fragNormal));
        vec3 nt = texture(texSampler[material.normalTexture], fragUV).rgb;
        normal = nt * 2.0 - 1.0; // convert normal from 0 to 1 range to -1 to 1 range
        normal = normalize(TBN * normal);
    }
    else
    {
        normal = normalize(fragNormal);
    }

    if (material.metallicRoughnessTexture != -1)
        metalRoughness = texture(texSampler[material.metallicRoughnessTexture], fragUV);
    else
        metalRoughness = vec4(material.metallicFactor, material.roughnessFactor, 1.0, 1.0);

    if (material.occlusionTexture != -1)
        AO = texture(texSampler[material.occlusionTexture], fragUV).r;
    else
        AO = material.occlusionStrength;

    if (material.emissiveTexture != -1)
    {
        emissive = texture(texSampler[material.emissiveTexture], fragUV).rgb;
        emissive.rgb = pow(emissive.rgb, vec3(2.2));
    }
    else
        emissive = vec3(0.0);
}

void main()
{
    vec4 albedo;
    vec3 normal;
    vec4 metalRoughness;
    float AO;
    vec3 emissive;
    setValues(albedo, normal, metalRoughness, AO, emissive);

    float thres = dither[int(mod(gl_FragCoord.y, 4.0)) * 4 + int(mod(gl_FragCoord.x, 4.0))];
    if (albedo.a < thres) discard;

    outAlbedo = vec4(albedo.rgb, 1.0);
    normal = normal * 0.5 + 0.5; // pack normal from -1 to 1 range to 0 to 1 range
    outNormal = vec4(normal, 1.0);
    outMetalRoughnessAO = vec4(metalRoughness.rgb, AO);
    outEmissive = vec4(emissive, 1.0);
}