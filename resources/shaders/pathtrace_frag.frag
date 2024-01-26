#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor; // index of framebuffer / attachment

layout(push_constant) uniform constants
{
    float time;
} pushConstants;

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct Material
{
    vec3 color;
    float emissionStrength;
    float roughness;
};

struct Sphere
{
    vec3 center;
    float radius;
    Material material;
};

struct HitInfo
{
    bool didHit;
    float dst;
    vec3 hitPoint;
    vec3 normal;
    Material material;
};

vec3 background(Ray ray)
{
    float a = 0.5 * (ray.direction.y + 1.0); // convert -1 (top) to 1 (bottom) range to 0 to 1
    return mix(vec3(0.5, 0.7, 1.0) * 0.01, vec3(0.0), a); // blend between white and blue linearly
}

HitInfo hitSphere(Sphere sphere, Ray ray)
{
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float half_b = dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = half_b * half_b - a * c;

    HitInfo hitInfo;
    hitInfo.didHit = false;
    if (discriminant >= 0.0)
    {
        float dst = (-half_b - sqrt(discriminant) ) / a; // distance along vector we hit
        if (dst >= 0.0)
        {
            hitInfo.didHit = true;
            hitInfo.dst = dst;
            hitInfo.hitPoint = ray.origin + dst * ray.direction;
            hitInfo.normal = normalize(hitInfo.hitPoint - sphere.center);
            hitInfo.material = sphere.material;
        }
    }
    return hitInfo;
}

const vec3 eye = vec3(0.0, 0.0, -0.1);
// arrange 3 spheres overlapping each other
Sphere spheres[5] = Sphere[](
Sphere(vec3(0.0, 0.0, 1.0), 0.25, Material(vec3(1.0, 0.0, 0.0), 1.0, 1.0)),
Sphere(vec3(0.6, 0.0, 1.0), 0.25, Material(vec3(0.0, 1.0, 0.0), 0.5, 1.0)),
Sphere(vec3(-0.6, 0.0, 1.0), 0.25, Material(vec3(0.0, 0.0, 1.0), 0.5, 1.0)),
Sphere(vec3(0.0, 0.0, 3.0), 2.0, Material(vec3(1.0), 0.0, 1.0)),
Sphere(vec3(0.0, 100.5, 1.0), 100, Material(vec3(1.0), 0.0, 1.0))
);

vec3 randomHemisphereVector(vec3 normal)
{
    float time = pushConstants.time;
    float u1 = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
    float u2 = fract(sin(gl_FragCoord.x * 54.8763 + gl_FragCoord.y * 24.9874) * 87954.1245);

    float r = sqrt(1.0 - u1 * u1);
    float phi = 2.0 * 3.14159265359 * u2;
    float x = r * cos(phi);
    float y = r * sin(phi);
    float z = u1;

    // Transform the local hemisphere coordinates to world coordinates
    vec3 tangent = normalize(cross(vec3(0.0, 1.0, 0.0), normal));
    vec3 bitangent = cross(normal, tangent);

    return normalize(tangent * x + bitangent * y + normal * z);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

HitInfo rayCollide(Ray ray)
{
    HitInfo finalHit;
    finalHit.dst = 1.0 / 0.0; // infinity
    for (int i = 0; i < spheres.length(); i++)
    {
        HitInfo info = hitSphere(spheres[i], ray);
        if (info.didHit && info.dst < finalHit.dst)
        {
            finalHit = info;
        }
    }
    return finalHit;
}

vec3 trace(Ray ray)
{
    vec3 rayColor = vec3(1.0);
    vec3 incomingLight = vec3(0.0);

    bool hitAnything = false;
    const int maxBounces = 3;
    for (int i = 0; i < maxBounces; i++)
    {
        HitInfo hitInfo = rayCollide(ray);
        if (hitInfo.didHit)
        {
            hitAnything = true;
            // find new ray direction and origin
            ray.origin = hitInfo.hitPoint;
            ray.direction = randomHemisphereVector(hitInfo.normal);

            Material material = hitInfo.material;

            /*if (hitInfo.material.roughness < 1.0)
                ray.direction = mix(reflect(ray.direction, hitInfo.normal), ray.direction, hitInfo.material.roughness);*/


            vec3 emissive = material.color * material.emissionStrength;
            incomingLight += emissive * rayColor;
            rayColor *= material.color;
        }
        else
        {
            incomingLight += rayColor * background(ray);
        }
    }
    if (hitAnything) return incomingLight;
    return background(ray);
}

void main()
{
    vec2 ndc = uv * 2.0f - 1.0f;

    // render across z = 1 plane / 90 FOV
    vec3 planeTarget = vec3(ndc, 1.0);
    // calculate aspect ratio and scale y by it
    float aspect = float(1024) / float(768);
    planeTarget.y /= aspect;

    for (int i = 0; i <= 2; i++)
    {
        spheres[i].center.y = sin(pushConstants.time * 2.0 + spheres[i].center.x) * 0.1;
        spheres[i].center.z = cos(pushConstants.time * 2.0 + spheres[i].center.x) * 0.05 + 1.0;
        spheres[i].material.color = vec3(0.0);
        spheres[i].material.color[i] = sin(pushConstants.time * 2.0 + spheres[i].center.x) * 0.5 + 0.5;
    }

    Ray ray;
    ray.origin = eye;
    ray.direction = normalize(planeTarget - eye);

    outColor = vec4(trace(ray), 1.0);
}