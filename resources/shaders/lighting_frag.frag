#version 450
#extension GL_EXT_scalar_block_layout : require

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inputAlbedo;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inputMetalRoughnessAO;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inputEmissive;
layout (input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput depthBuffer;

// radiance is the one with mip levels
layout(set = 2, binding = 0) uniform samplerCube cubeMap;
layout(set = 2, binding = 1) uniform samplerCube irradianceMap;
layout(set = 2, binding = 2) uniform samplerCube radianceMap;
layout(set = 2, binding = 3) uniform sampler2D   brdfLUT;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 cameraPos;
layout(location = 2) in vec3 cameraForward;
layout(location = 3) in mat4 cameraViewProj;

layout(scalar, push_constant) uniform LightingInfo {
    float iblIntensity;
    float emissionMultiplier;
    int pointLightCount;
    int sphereLightCount;
    int tubeLightCount;
    int rectangleLightCount;
} lightingInfo;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

struct Light
{
    vec3 position;
    vec3 color;
};

struct PointLight
{
    vec3 position;
    vec3 color;
    float intensity;
};

struct SphereLight
{
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

struct TubeLight
{
    vec3 position1;
    vec3 position2;
    vec3 color1;
    vec3 color2;
    float radius;
};

layout(scalar, set = 3, binding = 0) readonly buffer SphereLightBuffer {
    SphereLight sphereLights[];
} sphereLightBuffer;

layout(scalar, set = 3, binding = 1) readonly buffer TubeLightBuffer {
    TubeLight tubeLights[];
} tubeLightBuffer;

const float PI = 3.14159;

float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
    float a2 = pow(roughness, 4.0);
    float nwh2 = pow(dot(normal, halfway), 2.0);
    return a2 / (PI * pow(nwh2 * (a2 - 1.0) + 1, 2.0));
}
float GeometrySchlickGGX(float cosw, float roughness)
{
    float k = pow((roughness + 1.0), 2.0) / 8.0;
    return cosw / (cosw * (1.0 - k) + k);
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
vec4 reconstructPosition(float depth)
{
    // vulkan depth is 0 to 1
    vec4 ndc = vec4(fragUV * 2.0 - 1.0, depth, 1.0);
    vec4 pos = inverse(cameraViewProj) * ndc;
    pos /= pos.w;
    return pos;
}

struct Material
{
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
};

void calculateForLight(inout vec3 Lo, Light light, vec3 N, vec3 V, Material material, vec3 fragPos)
{
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    float distance = distance(light.position, fragPos);
    float attenuation = 1.0 / (distance * distance);

    vec3 radiance = light.color * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, material.roughness);
    float G   = GeometrySmith(N, V, L, material.roughness);
    vec3 F    = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, material.roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;
}

vec3 closestPointSphere(SphereLight light, vec3 R, vec3 fragPos)
{
    vec3 L = light.position.xyz - fragPos;
    vec3 centerToRay = (dot(L, R) * R) - L;
    vec3 closestPoint = L + centerToRay * clamp(light.radius / length(centerToRay), 0.0, 1.0);
    return closestPoint;
}

vec3 closestPointTube(TubeLight light, vec3 R, vec3 fragPos, out float t)
{
    vec3 lineToPoint = fragPos - light.position1;
    vec3 lineVector = light.position2 - light.position1;
    t = dot(lineToPoint, lineVector) / dot(lineVector, lineVector);
    t = clamp(t, 0.0, 1.0);

    vec3 closestPointLine = light.position1 + lineVector * t;

    SphereLight s;
    s.radius = light.radius;
    s.position = closestPointLine;
    return closestPointSphere(s, R, fragPos);
}

vec3 calculateIBL(vec3 N, vec3 V, vec3 R, vec3 F0, Material material)
{
    float nvDOT = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlickRoughness(nvDOT, F0, material.roughness);

    vec3 kD = 1.0 - F;
    kD *= 1.0 - material.metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * material.albedo;

    float MAX_REFLECTION_LOD = textureQueryLevels(radianceMap) + 1.0;
    float level = material.roughness * MAX_REFLECTION_LOD;
    vec3 prefilteredColor;
    if (level <= 1)
        prefilteredColor = texture(cubeMap, R).rgb;
    else
        prefilteredColor = textureLod(radianceMap, R, material.roughness * MAX_REFLECTION_LOD).rgb;
    vec2 envBRDF  = texture(brdfLUT, vec2(nvDOT, material.roughness)).rg;
    vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    vec3 ambient = (kD * diffuse + specular) * material.ao;
    return ambient;
}

const float[16] dither =
{
1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
};

void main()
{
    float depth = subpassLoad(depthBuffer).r;
    vec4 position = reconstructPosition(depth);

    vec4 albedo = subpassLoad(inputAlbedo);
    vec3 normal = subpassLoad(inputNormal).xyz;
    vec4 metalRoughnessAO = subpassLoad(inputMetalRoughnessAO);
    vec4 emissive = subpassLoad(inputEmissive);

    if (depth == 1.0)
    {
        outColor = vec4(0.0);
        return;
    }

    float metallic = metalRoughnessAO.b;
    float roughness = metalRoughnessAO.g;
    float ao = metalRoughnessAO.a;

    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos.xyz - position.xyz);
    vec3 R = reflect(-V, N); // reflection vector

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

    Material material = Material(albedo.rgb, metallic, roughness, ao);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lightingInfo.sphereLightCount; ++i)
    {
        SphereLight sphereLight = sphereLightBuffer.sphereLights[i];
        Light light = Light(closestPointSphere(sphereLight, R, position.xyz), sphereLight.color * sphereLight.intensity);
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }
    for (int i = 0; i < lightingInfo.tubeLightCount; ++i)
    {
        TubeLight tubeLight = tubeLightBuffer.tubeLights[i];
        float t;
        vec3 c = closestPointTube(tubeLight, R, position.xyz, t);
        Light light = Light(c, mix(tubeLight.color1, tubeLight.color2, t));
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }
//    for(int i = 0; i < 1; ++i)
//    {
//        Light light = Light(pointLights[i].position.xyz, pointLights[i].color.rgb, pointLights[i].color.a);
//        calculateForLight(Lo, light, N, V, material, position.xyz);
//    }

    vec3 ambient = calculateIBL(N, V, R, F0, material) * lightingInfo.iblIntensity;
    vec3 color = ambient + Lo + emissive.rgb * lightingInfo.emissionMultiplier;

    outColor = vec4(color.xyz, 1.0);
}