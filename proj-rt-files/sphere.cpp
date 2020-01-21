#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    TODO;
    return {0,0,0};
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    vec3 normal;
    TODO; // compute the normal direction
    return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    //TODO; // calculate bounding box
    // return box;
    Box box;
    box.hi = center + vec3( radius, radius, radius );
    box.lo = center - vec3( radius, radius, radius );
    return box;
}
