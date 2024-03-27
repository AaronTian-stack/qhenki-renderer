#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 lightV;

layout(location = 0) out vec4 outColor; // index of framebuffer / attachment

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main()
{
    // lambert
    float dot = dot(normalize(lightV), normalize(normal));
    float diff = clamp(dot, 0.0, 1.0);
    //outColor = vec4((normal + vec3(1.0)) * 0.5, 1.0);
    //outColor = vec4(fragUV, 0.0, 1.0);
    //outColor = vec4(fragColor * diff, 1.0);
    outColor = vec4(texture(texSampler, fragUV).rgb, 1.0);
}