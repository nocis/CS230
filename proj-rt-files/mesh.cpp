#include "mesh.h"
#include <fstream>
#include <string>
#include <limits>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts).
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts=triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{
    //TODO;
    double dist = 0.0;
    auto* obj = this;
    if ( !Intersect_Triangle( ray, part, dist ) )
        obj = nullptr;

    return { obj, dist, part };
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3& point, int part) const
{
    assert(part>=0);
    //TODO;
    return cross( vertices[ triangles[part][1] ] - vertices[ triangles[part][0] ], vertices[ triangles[part][2] ] - vertices[ triangles[part][0] ] ).normalized();
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray& ray, int tri, double& dist) const
{
    TODO;
    // (P - A) = b( B - A ) + c( C - A ) - su
    ivec3 triIdx = triangles[tri];
    vec3 A = vertices[ triIdx[0] ];
    vec3 B_A = vertices[ triIdx[1] ] - A;
    vec3 C_A = vertices[ triIdx[2] ] - A;
    vec3 P_A = ray.endpoint - A;

    //dot( ( u x v ), u ) = dot( ( u x v ), v ) = 0, order is crucial
    vec3 VxW = cross( B_A, C_A );
    vec3 UxV = cross( ray.direction, B_A );
    vec3 WxU = cross( C_A, ray.direction );

    //volume
    double volume =  dot( VxW, ray.direction );

    if ( fabs( volume ) < small_t )
        return false;

    double s = -dot( VxW, P_A ) / volume;
    double b = dot( WxU, P_A ) / volume;
    double c = dot( UxV, P_A ) / volume;

    //front and tri-inside check
    if ( s < small_t || b < -weight_tolerance || c < -weight_tolerance || 1.0 - b - c < -weight_tolerance )
        return false;

    dist = s;
    return true;
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const
{
    Box b;
    //TODO;
    b.Include_Point( vertices[ triangles[part][0] ] );
    b.Include_Point( vertices[ triangles[part][1] ] );
    b.Include_Point( vertices[ triangles[part][2] ] );
    return b;
}

bool Mesh::cullingTest(vec3 cameraLook, int part) const
{
    vec3 B_A = vertices[ triangles[part][1] ] - vertices[ triangles[part][0] ];
    vec3 C_A = vertices[ triangles[part][2] ] - vertices[ triangles[part][0] ];
    vec3 up = cross( B_A, C_A );
    return dot( up, cameraLook ) < 0;
}
