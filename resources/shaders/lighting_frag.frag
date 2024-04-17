#version 450

layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput inputAlbedo;
layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput inputMetalRoughnessAO;
layout (input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput inputEmissive;
layout (input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput depthBuffer;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 cameraPos;
layout(location = 2) in vec4 cameraForward;
layout(location = 3) in mat4 cameraViewProj;

layout(location = 0) out vec4 outColor; // location is index of framebuffer / attachment

struct Light
{
    vec3 position;
    vec3 color;
    float intensity;
};

struct PointLight
{
    vec4 position;
    vec4 color; // last parameter is intensity
};

struct SphereLight
{
    vec4 position; // last parameter is radius
    vec4 color; // last parameter is intensity
};

PointLight pointLights[1] = PointLight[1](
    PointLight(vec4(0.0, 0.0, 5.0, 1.0), vec4(0.0, 1.0, 0.0, 10.0))
);

SphereLight sphereLights[1] = SphereLight[1](
    SphereLight(vec4(0.0, 0.0, -5.0, 3.0), vec4(1.0, 0.9, 1.0, 10.0))
);

const float PI = 3.14159f;

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
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec4 reconstructPosition(float depth)
{
    // depth is 0 to 1 because vulkan
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
};

void calculateForLight(inout vec3 Lo, Light light, vec3 N, vec3 V, Material material, vec3 fragPos)
{
    vec3 F0 = mix(vec3(0.04), material.albedo, material.metallic);

    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(V + L);
    float distance    = distance(light.position, fragPos);
    float attenuation = 1.0 / (distance * distance);

    vec3 lightColor = light.color * light.intensity;
    vec3 radiance   = lightColor * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, material.roughness);
    float G   = GeometrySmith(N, V, L, material.roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

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

vec3 closestPointSphere(SphereLight light, vec3 N, vec3 V, vec3 fragPos)
{
    vec3 r = reflect(-V, N);
    vec3 L = light.position.xyz - fragPos;
    vec3 centerToRay = (dot(L, r) * r) - L;
    float sphereRadius = light.position.w;
    vec3 closestPoint = L + centerToRay * clamp(sphereRadius / length(centerToRay), 0, 1);
    return closestPoint;
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
    vec3 normalTexture = subpassLoad(inputNormal).xyz;
    vec4 metalRoughnessAO = subpassLoad(inputMetalRoughnessAO);
    vec4 emissive = subpassLoad(inputEmissive);

    if (depth == 1.0)
    {
        outColor = albedo;
        return;
    }

    float metallic = metalRoughnessAO.b;
    float roughness = metalRoughnessAO.g;
    float ao = metalRoughnessAO.a;

    vec3 normal = normalTexture * 2.0 - 1.0; // transform from [0, 1] to [-1, 1]
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos.xyz - position.xyz);

    vec3 F0 = mix(vec3(0.04), albedo.rgb, metallic);

    Material material = Material(albedo.rgb, metallic, roughness);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 1; ++i)
    {
        Light light = Light(closestPointSphere(sphereLights[i], N, V, position.xyz), sphereLights[i].color.rgb, sphereLights[i].color.a);
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }
    for(int i = 0; i < 1; ++i)
    {
        Light light = Light(pointLights[i].position.xyz, pointLights[i].color.rgb, pointLights[i].color.a);
        calculateForLight(Lo, light, N, V, material, position.xyz);
    }

    vec3 ambient = vec3(0.03) * albedo.rgb * ao;
    vec3 color = ambient + Lo + emissive.rgb;

    // TODO: use dithering to stop banding

//    float d = dither[int(mod(gl_FragCoord.y, 4.0)) * 4 + int(mod(gl_FragCoord.x, 4.0))];
//    color += vec3(d / 32.0 - (1.0 / 128.0));

    // TODO: move tonemapping to post process shader. store this output in 16f
    color = color / (color + vec3(1.0));

    outColor = vec4(color.xyz, 1.0);

    // basic blinn phong as debug for normals
//    vec3 lightDir = normalize(vec3(0.0, 0.0, 1.0) - position.xyz);
//    float diff = max(dot(N, lightDir), 0.0) + 0.2;
//    vec3 diffuse = diff * albedo.rgb;
//    outColor = vec4(diffuse, 1.0);
}