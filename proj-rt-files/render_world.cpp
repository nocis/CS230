#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0), skybox( nullptr ), ambient_intensity(0), enable_shadows(true),
    recursion_depth_limit(3)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
    for(size_t i=0;i<shaders.size();i++) delete shaders[i];
    for(size_t i=0;i<textures.size();i++) delete [] textures[i];
    delete [] skybox;
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    //sphere has no back face, triangle and plane have back face!
    //TODO;
    //bounding box collision detection
    Hit closestInfo = { nullptr, 0, 0 };
    std::vector<int> results;
    hierarchy.Intersection_Candidates( ray, results );

    //std::cout<<"        "<<std::endl;
    //ray casting
    for (int result : results)
    {
        Hit hitInfo = hierarchy.entries[result].obj->Intersection( ray, hierarchy.entries[result].part );
        if ( hitInfo.object && ( !closestInfo.object || hitInfo.dist < closestInfo.dist ) )
            closestInfo = hitInfo;
    }
    return closestInfo;
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    //TODO; // set up the initial view ray here
    //Ray ray;

    //remember normalized
    Ray ray( camera.position , ( camera.World_Position( pixel_index ) - camera.position ).normalized() );
#ifdef _DEPTH
    double depth = fmin( Cast_Ray_Depth(ray,1), 100 );
    if ( depth > small_t )
    {
        camera.grayMax = depth > camera.grayMax? depth : camera.grayMax;
        camera.grayMin = depth < camera.grayMin? depth : camera.grayMin;
        camera.Set_Depth( pixel_index, depth );
    }
#endif
    vec3 color=Cast_Ray( ray,recursion_depth_limit );
    camera.Set_Pixel( pixel_index,Pixel_Color( color ) );
    //std::cout<<pixel_index<<std::endl;
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy();

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));


#ifdef _DEPTH
    if (camera.grayMax != 0 && camera.grayMin < camera.grayMax)
    {
        for(int j=0;j<camera.number_pixels[1];j++)
        {
            for(int i=0;i<camera.number_pixels[0];i++)
            {
                camera.grayColors[j*camera.number_pixels[0]+i] -= camera.grayMax;
                camera.grayColors[j*camera.number_pixels[0]+i] /= ( camera.grayMin - camera.grayMax);
                camera.grayColors[j*camera.number_pixels[0]+i] *= 65535.0;
                if ( camera.grayColors[j*camera.number_pixels[0]+i] > 65535.0)
                    camera.grayColors[j*camera.number_pixels[0]+i] = 0;
            }
        }
    }
#endif
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;
    //TODO; // determine the color here
    Hit hitInfo = Closest_Intersection(ray);
    if ( hitInfo.object )
    {
        vec3 intersectionPoint = ray.Point( hitInfo.dist );
        vec3 normal = hitInfo.object->Normal( intersectionPoint, hitInfo.part );
        color = hitInfo.object->material_shader->Shade_Surface( ray, intersectionPoint, normal, recursion_depth );
        if ( !textures.empty() && textures[ hitInfo.object->texture_id ] )
        {
            color *= hitInfo.object->sampleTexture( hitInfo.part, intersectionPoint, textures[ hitInfo.object->texture_id ] );
        }
    }
    else
    {
        if ( !skybox )
            color = background_shader->Shade_Surface( Ray(), vec3(), vec3(), recursion_depth );
        else
        {
            vec3 dir =ray.direction;
            double a = dir[0];
            double b = dir[1];
            double c = dir[2];

            int axis = 0;
            double length = 0;

            if ( fabs( a ) > fmax( fabs( b ), fabs( c ) ) )
            {
                if ( a > 0 )
                    axis = 1;
                else
                    axis = -1;

                length = fabs(a);
            }
            else if ( fabs( b ) > fmax( fabs( a ), fabs( c ) ) )
            {
                if ( b > 0 )
                    axis = 2;
                else
                    axis = -2;

                length = fabs(b);
            }
            else if ( fabs( c ) > fmax( fabs( b ), fabs( a ) ) )
            {
                if ( c > 0 )
                    axis = 3;
                else
                    axis = -3;

                length = fabs(c);
            }

            a /= length;
            b /= length;
            c /= length;

            a += 1;
            b += 1;
            c += 1;

            a *= 512;
            b *= 512;
            c *= 512;

            int u = 0;
            int v = 0;
            ivec2 leftbottomPos;
            switch(axis)
            {
                //right
                case 1:
                    leftbottomPos = { 3072,1024 };
                    u = c;
                    v = b;
                    break;
                //left
                case -1:
                    leftbottomPos = { 2048,1024 };
                    u = -c;
                    v = b;
                    break;
                //up
                case 2:
                    leftbottomPos = { 2048,2048 };
                    u = -c;
                    v = a;
                    break;
                //down
                case -2:
                    leftbottomPos = { 2048,1024 };
                    u = -c;
                    v = -a;
                    break;
                //back
                case 3:
                    leftbottomPos = { 1024,1024 };
                    u = -a;
                    v = b;
                    break;
                //front
                case -3:
                    leftbottomPos = { 2048,1024 };
                    u = a;
                    v = b;
                    break;
            }

            Pixel pix = skybox[ ( leftbottomPos[1] + v ) * 4096 + ( leftbottomPos[0] + u ) ];
            color = From_Pixel(pix);
        }
    }

    return color;
}

double Render_World::Cast_Ray_Depth(const Ray& ray,int recursion_depth)
{
    //TODO; // determine the color heres
    Hit hitInfo = Closest_Intersection(ray);
    return hitInfo.dist;
}

void Render_World::Initialize_Hierarchy()
{
    // TODO; // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.
    unsigned int tmpSize = 0;
    for ( auto & tmpObj : objects )
    {
        tmpSize += tmpObj->number_parts;
    }
    //allocate before to improve performance
    hierarchy.entries.resize( tmpSize );
    hierarchy.tree.resize( 2 * tmpSize - 1 );
    hierarchy.rightChildOffset.resize( 2 * tmpSize - 1 );
    hierarchy.leaves.resize( 2 * tmpSize - 1 );
    hierarchy.entries_size = 0;
    for ( auto & tmpObj : objects )
    {
        for ( int j = 0; j < tmpObj->number_parts; j++ )
        {
            //backface culling
            if ( j < 1 || tmpObj->cullingTest( camera.look_vector, j ) || !enable_backface )
            {
                hierarchy.entries[ hierarchy.entries_size ] = Entry{ tmpObj, j, tmpObj->Bounding_Box(j) };
                hierarchy.entries_size++;
            }
        }
    }
    //Reorder_Entries every time when compute box
    //hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
    /*for ( auto & item : hierarchy.tree )
        std::cout<<item.lo<<" "<<item.hi<<std::endl;
    for ( auto & item : hierarchy.rightChildOffset )
        std::cout<<item<<" ";*/
}