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

void Hierarchy::Reorder_Entries( unsigned int begin, unsigned int  end )
{
    //BVH
    if ( begin >= end ) return;
    //TODO;
    vec3 center = ( entries[begin].box.lo + entries[begin].box.hi ) / 2.0;
    double max_x = center[0];
    double max_y = center[1];
    double max_z = center[2];

    double min_x = center[0];
    double min_y = center[1];
    double min_z = center[2];

    for ( unsigned int i = begin; i < end; i++ )
    {
        vec3 tmpCenter = ( entries[i].box.lo + entries[i].box.hi ) / 2.0;
        max_x = fmax( max_x, tmpCenter[0] );
        max_y = fmax( max_y, tmpCenter[1] );
        max_z = fmax( max_z, tmpCenter[2] );
        min_x = fmin( min_x, tmpCenter[0] );
        min_y = fmin( min_y, tmpCenter[1] );
        min_z = fmin( min_z, tmpCenter[2] );
    }

    if ( ( max_x - min_x ) > fmax( max_y - min_y, max_z - min_z ) - small_t )
    {
        axis = 0;
        maxLength = max_x - min_x;
        maxPos = max_x;
        minPos = min_x;
    }
    else if ( ( max_y - min_y ) > fmax( max_x - min_x, max_z - min_z ) - small_t )
    {
        axis = 1;
        maxLength = max_y - min_y;
        maxPos = max_y;
        minPos = min_y;
    }
    else
    {
        axis = 2;
        maxLength = max_z - min_z;
        maxPos = max_z;
        minPos = min_z;
    }

    //sort by axis
    std::sort( entries.begin() + begin, entries.begin() + end, compare_help );
}

void Hierarchy::Compute_Box( unsigned int start, unsigned int end, Box& box )
{
    TODO;
    if ( start >= end )
        return;

    if ( box.lo[0] <= box.hi[0] )
    {
        tree[tree_size] = box;
        rightChildOffset[tree_size] = 0;
        tree_size++;
    }

    if ( start == end - 1 )
    {
        leaves[ tree_size - 1 ] = leavesCount;
        leavesCount++;
        return;
    }

    Reorder_Entries( start, end );

    double startAxis = ( entries[start].box.lo[axis] + entries[start].box.hi[axis] ) / 2.0 - small_t;
    double endAxis = ( entries[end - 1].box.lo[axis] + entries[end - 1].box.hi[axis] ) / 2.0 + small_t;
    double lengthAxis = endAxis - startAxis;
    double strideAxis = lengthAxis / 4.0;

    double expectationMin = 0x3f3f3f3f;
    unsigned int expectationDivide = 0x3f3f3f3f;

    int leftCount = 0;
    int rightCount = 0;

    Box leftBox, rightBox;

    for ( int i = 0; i <= 4; i++ )
    {
        leftBox.Make_Empty();
        rightBox.Make_Empty();

        unsigned int divide = start;

        leftCount = 0;
        rightCount = 0;

        for ( unsigned int j = start; j < end; j++ )
        {
            double entryCenter = ( entries[j].box.hi[axis] + entries[j].box.lo[axis] ) / 2.0 ;

            if ( entryCenter <= startAxis + i * strideAxis )
            {
                divide = j;
                leftCount++;
                leftBox = leftBox.Union( entries[j].box );
            }
            else
            {
                rightCount++;
                rightBox = rightBox.Union( entries[j].box );
            }
        }

        double expectationCost = leftBox.SurfaceArea() / box.SurfaceArea() * leftCount + rightBox.SurfaceArea() / box.SurfaceArea() * rightCount;
        if ( expectationCost < expectationMin - small_t )
        {
            expectationDivide = divide + 1;
            expectationMin = expectationCost;
        }
    }

    leftBox.Make_Empty();
    rightBox.Make_Empty();
    //std::cout<<expectationDivide<<std::endl;
    if ( ( expectationDivide == 0x3f3f3f3f || expectationDivide == start || expectationDivide == end ) && start < end )
        expectationDivide = ( start + end ) / 2;

    for ( unsigned int i = start; i < end; i++ )
    {
        if ( i < expectationDivide )
            leftBox = leftBox.Union( entries[i].box );
        else
            rightBox = rightBox.Union( entries[i].box );
    }

    unsigned int parentIndex = tree_size - 1;
    Compute_Box( start, expectationDivide, leftBox);
    unsigned int rightChildIndex = tree_size;
    Compute_Box( expectationDivide, end, rightBox);
    rightChildOffset[parentIndex] = rightChildIndex;
}

// Populate tree from entries.
void Hierarchy::Build_Tree()
{
    if ( entries.empty() ) return;
    TODO;
    tree_size = 0;
    leavesCount = 0;
    Box box;
    box.Make_Empty();
    for ( auto & item : entries )
    {
        box = box.Union(item.box);
    }

    Compute_Box( 0, entries_size, box );
}

// Return a list of candidates (indices into the entries list) whose
// bounding boxes intersect the ray.
void Hierarchy::Intersection_Candidates( const Ray& ray, std::vector<int>& candidates, unsigned int root) const
{
    //1. tree must have both left node and right node, if rightChildOffset[root] == 0, it must be a leaf
    TODO;
    if ( !tree[root].Intersection( ray ) )
        return;

    if ( !rightChildOffset[root] )
    {
        //std::cout<<entries[leaves[root]].part<<":"<<entries[leaves[root]].box.lo<<"         "<<entries[leaves[root]].box.hi<<std::endl;
        candidates.push_back( leaves[root] );
        return;
    }

    //travel left
    if ( rightChildOffset[root] > root + 1 )
    {
        Intersection_Candidates( ray, candidates, root + 1 );
    }

    //travel right
    Intersection_Candidates( ray, candidates, rightChildOffset[root] );
}

