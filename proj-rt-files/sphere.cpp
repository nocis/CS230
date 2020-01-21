#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{
    TODO;
    double solution1 = 0.0;
    double solution2 = 0.0;
    double DIST = 0.0;
    vec3 DIS_C_TO_E = ray.endpoint - center;
    double DIS_T_DIR = dot( DIS_C_TO_E, ray.direction );
    double DIR_T_DIR = dot( ray.direction, ray.direction );
    double DIS_T_DIS = dot( DIS_C_TO_E, DIS_C_TO_E );

    double radius_2 = radius * radius; //multiplication is faster than pow()
    double root = DIR_T_DIR * ( DIS_T_DIS - radius_2 );
    root = DIS_T_DIR * DIS_T_DIR - root;

    if ( root < 0 )
    {
        return {0,0,0};
    }
    else if ( root < small_t )
    {
        solution1 = -1.0 * DIS_T_DIR / DIR_T_DIR;
        solution2 = -1.0 * DIS_T_DIR / DIR_T_DIR;
    }
    else
    {
        root = 2.0 * sqrt( root );
        solution1 = (-2.0 * DIS_T_DIR + root) / 2.0 / DIR_T_DIR;
        solution2 = (-2.0 * DIS_T_DIR - root) / 2.0 / DIR_T_DIR;
    }

    if ( solution1 < 0.0 )
    {
        std::cout<<"Wrong computation!"<<std::endl;
        exit(0);
    }
    else if ( solution2 > small_t )
        DIST = solution2;
    else
        DIST = 0.0;

    return {this, DIST, part };
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    //vec3 normal;
    //TODO; // compute the normal direction

    return ( point - center ).normalized();
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
