// sphere
vec3 closestPointSphere(float radius, vec3 L, vec3 R, vec3 fragPos)
{
    vec3 centerToRay = (dot(L, R) * R) - L;
    vec3 closestPoint = L + centerToRay * clamp(radius / length(centerToRay), 0.0, 1.0);
    return normalize(closestPoint);
}

// tube
vec3 closestPointOnLine(vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    float t = dot(c - a , ab) / dot(ab, ab);
    return a + t * ab ;
}
vec3 closestPointOnSegment(vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    float t = dot(c - a, ab) / dot(ab, ab);
    return a + clamp (t, 0.0, 1.0) * ab;
}

// rectangle
vec3 tracePlane(vec3 planeOrigin, vec3 planeNormal, vec3 fragPos, vec3 R)
{
    float denom = dot(planeNormal, R);
    float t = dot(planeNormal, planeOrigin - fragPos) / denom;
    vec3 planeIntersectionPoint = fragPos + R * t;
    return planeIntersectionPoint;
}
vec3 closestPointOnRectangle(vec3 fragPos, vec3 planeOrigin, vec3 planeright, vec3 planeUp, vec2 halfSize)
{
    vec3 dir = fragPos - planeOrigin;
    vec2 dist2D = vec2(dot(dir, -planeright), dot(dir, planeUp));
    dist2D = clamp(dist2D, -halfSize, halfSize);
    return planeOrigin + planeright * dist2D.x + planeUp * dist2D.y;
}