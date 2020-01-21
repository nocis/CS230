#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    TODO;
    //bounding box collision detection
    std::vector<int> results;
    hierarchy.Intersection_Candidates(ray, results);
    //ray casting
    for( int i = 0; i < results.size(); i++ )
    {
        hierarchy.entries[i].obj->Intersection(ray, hierarchy.entries[i].part);
    }
    return {};
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    //TODO; // set up the initial view ray here
    //Ray ray;

    //remember normalized
    Ray ray( camera.position , ( camera.World_Position( pixel_index ) - camera.position ).normalized() );

    vec3 color=Cast_Ray(ray,1);
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy();

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;
    TODO; // determine the color here
    Hit hitInfo = Closest_Intersection(ray);
    return color;
}

void Render_World::Initialize_Hierarchy()
{
    // TODO; // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.

    for( int i = 0; i < objects.size(); i++ )
    {
        Object* tmpObj = objects[i];
        for ( int j = 0; j < tmpObj->number_parts; j++ )
        {
            hierarchy.entries.push_back(Entry{tmpObj, j, tmpObj->Bounding_Box(j)});
        }
    }
    hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
}
