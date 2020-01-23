#include <limits>
#include "box.h"

// Return whether the ray intersects this box.
bool Box::Intersection(const Ray& ray) const
{
    //Slabs method
    TODO;
    //return true;
    //right hand
    double endx = ray.endpoint[0];
    double endy = ray.endpoint[1];
    double endz = ray.endpoint[2];
    double dirx = ray.direction[0];
    double diry = ray.direction[1];
    double dirz = ray.direction[2];
    double x_left = lo[0];
    double x_right = hi[0];
    double y_left = lo[1];
    double y_right = hi[1];
    double z_left = lo[2];
    double z_right = hi[2];
    double x_in = 0.0;
    double x_out = 0.0;
    double y_in = 0.0;
    double y_out = 0.0;
    double z_in = 0.0;
    double z_out = 0.0;

    if ( fabs( dirx ) > 1e-10 )
    {
        if ( dirx > 0 )
        {
            x_in = ( x_left - endx ) / dirx;
            x_out = ( x_right - endx ) / dirx;
        }
        else
        {
            x_out = ( x_left - endx ) / dirx;
            x_in = ( x_right - endx ) / dirx;
        }
    }
    else if ( endx < x_left || endx > x_right )
        return false;

    if ( fabs( diry ) > 1e-10 )
    {
        if ( diry > 0 )
        {
            y_in = ( y_left - endy ) / diry;
            y_out = ( y_right - endy ) / diry;
        }
        else
        {
            y_out = ( y_left - endy ) / diry;
            y_in = ( y_right - endy ) / diry;
        }
    }
    else if ( endy < y_left || endy > y_right )
        return false;

    if ( fabs( dirz ) > 1e-10 )
    {
        if ( dirz > 0 )
        {
            z_in = ( z_left - endz ) / dirz;
            z_out = ( z_right - endz ) / dirz;
        }
        else
        {
            z_out = ( z_left - endz ) / dirz;
            z_in = ( z_right - endz ) / dirz;
        }
    }
    else if ( endz < z_left || endz > z_right )
        return false;

    return fmax( x_in, fmax( y_in, z_in ) ) < fmin( x_out, fmin( y_out, z_out ) );
}

// Compute the smallest box that contains both *this and bb.
Box Box::Union(const Box& bb) const
{
    Box box;
    TODO;
    box.lo[0] = fmin( lo[0], bb.lo[0] );
    box.lo[1] = fmin( lo[1], bb.lo[1] );
    box.lo[2] = fmin( lo[2], bb.lo[2] );
    box.hi[0] = fmax( hi[0], bb.hi[0] );
    box.hi[1] = fmax( hi[1], bb.hi[1] );
    box.hi[2] = fmax( hi[2], bb.hi[2] );
    return box;
}

// Enlarge this box (if necessary) so that pt also lies inside it.
void Box::Include_Point(const vec3& pt)
{
    TODO;
    lo[0] = fmin( pt[0], lo[0] );
    lo[1] = fmin( pt[1], lo[1] );
    lo[2] = fmin( pt[2], lo[2] );
    hi[0] = fmax( pt[0], hi[0] );
    hi[1] = fmax( pt[1], hi[1] );
    hi[2] = fmax( pt[2], hi[2] );
}

// Create a box to which points can be correctly added using Include_Point.
void Box::Make_Empty()
{
    lo.fill(std::numeric_limits<double>::infinity());
    hi=-lo;
}

double Box::SurfaceArea()
{
    if ( hi[0] < lo[0] )
        return 0;
    double xLengthBox = hi[0] - lo[0];
    double yLengthBox = hi[1] - lo[1];
    double zLengthBox = hi[2] - lo[2];
    return 2.0 *( xLengthBox * yLengthBox + xLengthBox * zLengthBox + zLengthBox * yLengthBox );
}

vec3 Box::Center() {
    return ( hi + lo ) / 2.0;
}
