#version 450

layout(std140, set = 0, binding = 0) uniform matrices {
    mat4 model;
    mat4 viewProj;
    //mat4 proj;
} ubo;

/*layout(binding = 1) uniform foo {
    mat4 model;
} foobar;*/

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

layout(location = 0) in vec2 inPosition; // note that some types use multiple slots
layout(location = 1) in vec3 inColor;

void main()
{
    gl_Position = ubo.viewProj * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}