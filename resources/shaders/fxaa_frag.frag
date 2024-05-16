#version 450
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

layout(scalar, push_constant) uniform PushConstant {
    vec2 viewport;
    float fxaaReduceMul;
    float fxaaReduceMin;
    float fxaaSpanMax;
} pc;

vec3 fxaa(vec2 texCoords, vec2 viewportInv)
{
    float u_fxaaReduceMul = 1.0 / pc.fxaaReduceMul;
    float u_fxaaReduceMin = 1.0 / pc.fxaaReduceMin;
    float u_fxaaSpanMax = pc.fxaaSpanMax;

    vec3 rgbNW = texture(texSampler, texCoords.xy + (vec2(-1.0, -1.0) * viewportInv)).xyz;
    vec3 rgbNE = texture(texSampler, texCoords.xy + (vec2(+1.0, -1.0) * viewportInv)).xyz;
    vec3 rgbSW = texture(texSampler, texCoords.xy + (vec2(-1.0, +1.0) * viewportInv)).xyz;
    vec3 rgbSE = texture(texSampler, texCoords.xy + (vec2(+1.0, +1.0) * viewportInv)).xyz;
    vec3 rgbN = texture(texSampler, texCoords.xy + (vec2(0.0, -1.0) * viewportInv)).xyz;
    vec3 rgbS = texture(texSampler, texCoords.xy + (vec2(0.0, 1.0) * viewportInv)).xyz;
    vec3 rgbE= texture(texSampler, texCoords.xy + (vec2(1.0, 0.0) * viewportInv)).xyz;
    vec3 rgbW= texture(texSampler, texCoords.xy + (vec2(-1.0, 0.0) * viewportInv)).xyz;
    vec3 rgbM = texture(texSampler, texCoords.xy).xyz;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaN = dot(rgbN, luma);
    float lumaS = dot(rgbS, luma);
    float lumaE = dot(rgbE, luma);
    float lumaW = dot(rgbW, luma);
    float lumaM = dot(rgbM, luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, min(lumaSE, min(lumaN, min(lumaS, min(lumaE, lumaW)))))));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, max(lumaSE, max(lumaN, max(lumaS, max(lumaE, lumaW)))))));

    vec2 dir;
    dir.x = abs(lumaN + lumaS - 2.0 * lumaM) * 2.0 +
    abs(lumaNE + lumaSE - 2.0 * lumaE) +
    abs(lumaNW + lumaSW - 2.0 * lumaW);
    dir.y = abs(lumaE + lumaW - 2.0 * lumaM) * 2.0 +
    abs(lumaNE + lumaNW - 2.0 * lumaN) +
    abs(lumaSE + lumaSW - 2.0 * lumaS);

    float dirReduce = max(
    (lumaNW + lumaNE + lumaSW + lumaSE + lumaN + lumaS + lumaE + lumaW) * (0.125 * u_fxaaReduceMul),
    u_fxaaReduceMin);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(u_fxaaSpanMax, u_fxaaSpanMax), max(vec2(-u_fxaaSpanMax, -u_fxaaSpanMax), dir * rcpDirMin)) * viewportInv;

    vec3 rgbA =	0.5	* (texture(texSampler, texCoords.xy + dir * (1.0 / 3.0 - 0.5)).xyz +
    texture(texSampler, texCoords.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB =	rgbA * 0.5 + 0.25 * (texture(texSampler, texCoords.xy + dir * (0.0 / 3.0 - 0.5)).xyz +
    texture(texSampler, texCoords.xy + dir * (3.0 / 3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    vec3 color = vec3(0.0);

    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        color = rgbA;
    } else {
        color = rgbB;
    }

    return color;
}

void main()
{
    vec2 viewportInv = vec2(1.0) / pc.viewport;
//    vec3 color = fxaa(fragUV, viewportInv);
    vec3 color = texture(texSampler, fragUV).rgb;
    outColor = vec4(mix(vec3(1.0, 0.0, 0.0), color, 0.5), 1.0);
}