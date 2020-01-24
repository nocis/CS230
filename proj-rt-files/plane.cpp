#include "plane.h"
#include "ray.h"
#include <cfloat>
#include <limits>

// Intersect with the half space defined by the plane.  The plane's normal
// points outside.  If the ray starts on the "inside" side of the plane, be sure
// to record a hit with t=0 as the first entry in hits.
Hit Plane::Intersection(const Ray& ray, int part) const
{
    //TODO;
    //dot(e+ut-p, normal) = 0

    double a = dot( ray.endpoint - x1, normal );
    double b = dot( ray.direction, normal );

    //back face test
    if ( fabs(b) < 1e-10 )
        return {0,0,0};

    double t = a / -b;

    if ( t < 0 )
        return {0,0,0};

    return { this, t, part };
}

vec3 Plane::Normal(const vec3& point, int part) const
{
    return normal;
}

// There is not a good answer for the bounding box of an infinite object.
// The safe thing to do is to return a box that contains everything.
Box Plane::Bounding_Box(int part) const
{
    Box b;
    b.hi.fill(std::numeric_limits<double>::max());
    b.lo=-b.hi;
    return b;
}
