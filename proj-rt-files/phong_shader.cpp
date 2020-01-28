#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::
Shade_Surface(const Ray& ray,const vec3& intersection_point,
    const vec3& normal,int recursion_depth) const
{
    vec3 color;
    TODO; //determine the color

    if ( !recursion_depth )
        return color;

    //multiplication for color blend, addition for intensity accumulation
    //object has color (r,g,b) max value = 1 for every slot, which indicates how much value from light( (1,1,1) is white light)
    //will be preserved, so multiplication implies such process.

    //ambient, which should from some ambient light( ( intensity * ambientLightColor ) blend with( multiply by ) objectColor ).
    //light invariant
    color += world.ambient_color * world.ambient_intensity * color_ambient;

    for ( auto light : world.lights )
    {
        //ignore lights who are behind of object
        double cosTheta = dot( ( light->position - intersection_point ).normalized(), normal );
        if ( cosTheta < 0.0 )
            continue;

        //shadow ray check, no self-shadow, no behind light shadow
        Hit shadowHit = world.Closest_Intersection( Ray( intersection_point, ( light->position - intersection_point ).normalized() ) );
        if ( world.enable_shadows && shadowHit.object && shadowHit.dist < ( light->position - intersection_point ).magnitude() && shadowHit.dist > small_t )
            continue;

        vec3 lightColor = light->Emitted_Light( light->position - intersection_point );

        //diffuse
        color += fmax( cosTheta, 0.0 ) * lightColor * color_diffuse;

        //specular
        //rayout = 2cos*n - rayin
        color += pow( fmax( dot( -ray.direction, 2 * cosTheta * normal - ( light->position - intersection_point ).normalized() ), 0.0 ), specular_power ) * lightColor * color_specular;
    }
    return color;
}
