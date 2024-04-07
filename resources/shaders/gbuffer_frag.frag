#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outPosition; // location is index of framebuffer / attachment
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormal;
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

const float dither[256] = float[](
    0, 191,  48, 239,  12, 203,  60, 251,   3, 194,  51, 242,  15, 206,  63, 254,
    127,  64, 175, 112, 139,  76, 187, 124, 130,  67, 178, 115, 142,  79, 190, 127,
    32, 223,  16, 207,  44, 235,  28, 219,  35, 226,  19, 210,  47, 238,  31, 222,
    159,  96, 143,  80, 171, 108, 155,  92, 162,  99, 146,  83, 174, 111, 158,  95,
    8, 199,  56, 247,   4, 195,  52, 243,  11, 202,  59, 250,   7, 198,  55, 246,
    135,  72, 183, 120, 131,  68, 179, 116, 138,  75, 186, 123, 134,  71, 182, 119,
    40, 231,  24, 215,  36, 227,  20, 211,  43, 234,  27, 218,  39, 230,  23, 214,
    167, 104, 151,  88, 163, 100, 147,  84, 170, 107, 154,  91, 166, 103, 150,  87,
    2, 193,  50, 241,  14, 205,  62, 253,   1, 192,  49, 240,  13, 204,  61, 252,
    129,  66, 177, 114, 141,  78, 189, 126, 128,  65, 176, 113, 140,  77, 188, 125,
    34, 225,  18, 209,  46, 237,  30, 221,  33, 224,  17, 208,  45, 236,  29, 220,
    161,  98, 145,  82, 173, 110, 157,  94, 160,  97, 144,  81, 172, 109, 156,  93,
    10, 201,  58, 249,   6, 197,  54, 245,   9, 200,  57, 248,   5, 196,  53, 244,
    137,  74, 185, 122, 133,  70, 181, 118, 136,  73, 184, 121, 132,  69, 180, 117,
    42, 233,  26, 217,  38, 229,  22, 213,  41, 232,  25, 216,  37, 228,  21, 212,
    169, 106, 153,  90, 165, 102, 149,  86, 168, 105, 152,  89, 164, 101, 148,  85
);

void setValues(out vec4 albedo, out vec3 normal, out vec2 metalRoughness, out float AO, out vec3 emissive)
{
    if (material.baseColorTexture != -1)
    {
        albedo = texture(texSampler[material.baseColorTexture], fragUV);
        albedo.rgb = pow(albedo.rgb, vec3(2.2));
    }
    else
        albedo = material.baseColorFactor;

//    if (material.normalTexture != -1)
//    {
//        // TODO: TBN calculation
//        normal = texture(texSampler[material.normalTexture], fragUV).rgb;
//    }
//    else
//    {
        normal = normalize(fragNormal);
        // turn into 0-1 range
        normal = normal + 1.0 * 0.5;
    //}

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
    vec4 albedo;
    vec3 normal;
    vec2 metalRoughness;
    float AO;
    vec3 emissive;
    setValues(albedo, normal, metalRoughness, AO, emissive);

    float thres = dither[int(mod(gl_FragCoord.y, 4.0)) * 16 + int(mod(gl_FragCoord.x, 4.0))] / 255.0;
    if (albedo.a < thres) discard;

    outPosition = vec4(fragPos, 1.0);
    outAlbedo = albedo;
    outNormal = vec4(normal, 1.0);
    outMetalRoughnessAO = vec4(metalRoughness, AO, 1.0);
}