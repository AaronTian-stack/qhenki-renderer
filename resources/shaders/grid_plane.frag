#version 450
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(location = 0) in vec3 camPos;
layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;
layout(location = 3) in vec2 fragUV;
layout(location = 4) in mat4 viewProj;

layout(scalar, set = 1, binding = 0) uniform DitherMatrix {
    float dither[64];
} ditherMatrix;

layout(scalar, push_constant) uniform PushConstant {
    float gridScale;
} pc;

float computeDepth(vec3 pos)
{
    vec4 ndc = viewProj * vec4(pos, 1.0);
    return (ndc.z / ndc.w);
}

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar - d * (zFar - zNear));
}

float filteredGrid(vec2 p, vec2 dpdx, vec2 dpdy)
{
    // thanks iq
    vec2 w = max(abs(dpdx), abs(dpdy));
    vec2 a = p + 0.5*w;
    vec2 b = p - 0.5*w;
    vec2 i = (floor(a)+min(fract(a)*pc.gridScale,1.0)-
    floor(b)-min(fract(b)*pc.gridScale,1.0))/(pc.gridScale*w);
    return (1.0-i.x)*(1.0-i.y);
}

void main()
{
    // only render the plane if above it
    if (nearPoint.y < 0.0) discard;
    // ray trace plane
    vec3 rayDir = normalize(farPoint - nearPoint);
    float t = -nearPoint.y / rayDir.y;
    vec3 hitPoint = nearPoint + t * rayDir;

    vec2 matuv = hitPoint.xz * 1.0;
    vec3 dx = dFdx(hitPoint);
    vec3 dy = dFdy(hitPoint);
    float mate = filteredGrid(matuv, dx.xz, dy.xz);

    float depth = computeDepth(hitPoint);
    gl_FragDepth = depth;

    float linearDepth = linearize_depth(depth, 0.1, 1000.0) / 1000.0;

    float dither = ditherMatrix.dither[int(mod(gl_FragCoord.y, 8.0)) * 8 + int(mod(gl_FragCoord.x, 8.0))];

    if (t < 0.0) //|| (1.0 - linearDepth) < dither)
        discard;

    vec4 color = vec4(1.0);
    // check if hitpoint is on x or z axis
    if (abs(hitPoint.z) < 0.1)
    {
        mate = 1.0;
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }
    if (abs(hitPoint.x) < 0.1)
    {
        mate = 1.0;
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }

    outColor = color * mate;
}