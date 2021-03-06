#ifndef __PLANE_H__
#define __PLANE_H__

#include "object.h"

class Plane : public Object
{
public:
    vec3 x1;
    vec3 normal;

    Plane(const vec3& point,const vec3& normal)
        :x1(point),normal(normal.normalized())
    {}

    virtual Hit Intersection(const Ray& ray, int part) const override;
    virtual vec3 Normal(const vec3& point, int part) const override;
    virtual Box Bounding_Box(int part) const override;
    bool cullingTest( vec3 cameraLook, int part ) const override { return true; }
    vec3 sampleTexture( int part, vec3 point, unsigned int* text ) const override {return {};}
};
#endif
