#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color;
    TODO; // determine the color

    if ( !recursion_depth )
        return color;

    //c0 + reflectivity*(cr-c0)

    //c0
    color = shader->Shade_Surface( ray, intersection_point, normal, recursion_depth );

    //cr
    color += ( world.Cast_Ray( Ray( intersection_point, ( ray.direction + 2 * dot( -ray.direction, normal ) * normal ).normalized() ), --recursion_depth ) - color ) * reflectivity;
    return color;
}
