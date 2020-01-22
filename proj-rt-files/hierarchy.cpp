#include <algorithm>
#include "hierarchy.h"

// Reorder the entries vector so that adjacent entries tend to be nearby.
// You may want to implement box.cpp first.
int Hierarchy::axis = 0;
bool Hierarchy::compare_help( const Entry& a, const Entry& b )
{
    double coord_a = ( a.box.hi[Hierarchy::axis] + a.box.lo[Hierarchy::axis] ) / 2.0;
    double coord_b = ( b.box.hi[Hierarchy::axis] + b.box.lo[Hierarchy::axis] ) / 2.0;
    return coord_a < coord_b;
}

void Hierarchy::Reorder_Entries()
{
    //BVH
    if ( !entries.size() ) return;
    //TODO;
    double max_x = entries[0].box.hi[0];
    double max_y = entries[0].box.hi[1];
    double max_z = entries[0].box.hi[2];

    double min_x = entries[0].box.lo[0];
    double min_y = entries[0].box.lo[1];
    double min_z = entries[0].box.lo[2];

    for ( int i = 1; i < entries.size(); i++ )
    {
        max_x = fmax( max_x, entries[i].box.hi[0] );
        max_y = fmax( max_y, entries[i].box.hi[1] );
        max_z = fmax( max_z, entries[i].box.hi[2] );
        min_x = fmin( min_x, entries[i].box.lo[0] );
        min_y = fmin( min_y, entries[i].box.lo[1] );
        min_z = fmin( min_z, entries[i].box.lo[2] );
    }

    if ( ( max_x - min_x ) > fmax( max_y - min_y, max_z - min_z ) )
    {
        axis = 0;
        maxLength = max_x - min_x;
    }
    else if ( ( max_y - min_y ) > fmax( max_x - min_x, max_z - min_z ) )
    {
        axis = 1;
        maxLength = max_y - min_y;
    }
    else
    {
        axis = 2;
        maxLength = max_z - min_z;
    }

    //sort by axis
    std::sort( entries.begin(), entries.end(), compare_help);
}

// Populate tree from entries.
void Hierarchy::Build_Tree()
{
    if ( !entries.size() ) return;
    TODO;
    tree.push_back( entries[0].box );
    tree.push_back( entries[1].box );
    tree.push_back( entries[2].box );
    tree.push_back( entries[3].box );
}

// Return a list of candidates (indices into the entries list) whose
// bounding boxes intersect the ray.
void Hierarchy::Intersection_Candidates(const Ray& ray, std::vector<int>& candidates) const
{
    TODO;
    candidates.push_back(0);
    candidates.push_back(1);
    candidates.push_back(2);
    candidates.push_back(3);

}
