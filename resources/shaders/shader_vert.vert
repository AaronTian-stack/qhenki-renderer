#version 450

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

vec2 positions[3] = vec2[](
vec2(-1.0, -1.0),
vec2(2.0, -1.0),
vec2(-1.0, 2.0)
);

vec3 colors[3] = vec3[](
vec3(1.0, 0.0, 0.0),
vec3(0.0, 1.0, 0.0),
vec3(0.0, 0.0, 1.0)
);

void main()
{
    uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0f + -1.0f, 0.0f, 1.0f);
    fragColor = colors[gl_VertexIndex];
}