#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 lightV;

layout(location = 0) out vec4 outColor; // index of framebuffer / attachment

void main()
{
    // lambert
    float dot = dot(normalize(lightV), normalize(normal));
    float diff = clamp(dot, 0.0, 1.0);
    //outColor = vec4((normal + vec3(1.0)) * 0.5, 1.0);
    //outColor = vec4(clamp(fragColor * diff, vec3(0.0), vec3(1.0)), 1.0);
    outColor = vec4(fragColor * diff, 1.0);
}