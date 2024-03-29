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

//layout(push_constant) uniform mats {
//    int index1;
//    int index2;
//    int index3;
//    int index4;
//    vec4 albedo;
//    float metal;
//    float roughness;
//} material;

void main()
{
    // lambert
    float dot = dot(normalize(lightV), normalize(normal));
    float diff = clamp(dot, 0.0, 1.0);

    vec3 normalV = normal + vec3(1.0) * 0.5;

    vec3 c = texture(texSampler[0], fragUV).rgb * diff;
    outColor = vec4(c, 1.0);
}