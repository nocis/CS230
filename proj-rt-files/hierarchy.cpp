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
    if ( entries.empty() ) return;
    //TODO;
    double max_x = entries[0].box.hi[0];
    double max_y = entries[0].box.hi[1];
    double max_z = entries[0].box.hi[2];

    double min_x = entries[0].box.lo[0];
    double min_y = entries[0].box.lo[1];
    double min_z = entries[0].box.lo[2];

    for ( auto & item : entries )
    {
        max_x = fmax( max_x, item.box.hi[0] );
        max_y = fmax( max_y, item.box.hi[1] );
        max_z = fmax( max_z, item.box.hi[2] );
        min_x = fmin( min_x, item.box.lo[0] );
        min_y = fmin( min_y, item.box.lo[1] );
        min_z = fmin( min_z, item.box.lo[2] );
    }

    if ( ( max_x - min_x ) > fmax( max_y - min_y, max_z - min_z ) )
    {
        axis = 0;
        maxLength = max_x - min_x;
        maxPos = max_x;
        minPos = min_x;
    }
    else if ( ( max_y - min_y ) > fmax( max_x - min_x, max_z - min_z ) )
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
    std::sort( entries.begin(), entries.end(), compare_help);
}

void Hierarchy::Compute_Box( int start, int end, Box& box )
{
    TODO;
    if ( start >= end )
        return;

    if ( box.lo[0] <= box.hi[0] )
    {
        tree.push_back( box );
        rightChildOffset.push_back(0);
    }


    if ( start == end - 1 )
        return;

    double startAxis = entries[start].box.lo[axis];
    double endAxis = entries[end - 1].box.hi[axis];
    double lengthAxis = endAxis - startAxis;
    double strideAxis = lengthAxis / 4.0;

    double expectationMin = 0x3f3f3f3f;
    int expectationDivide = 0x3f3f3f3f;

    int leftCount = 0;
    int rightCount = 0;

    Box leftBox, rightBox;

    for ( int i = 0; i <= 4; i++ )
    {
        leftBox.Make_Empty();
        rightBox.Make_Empty();

        int divide = start;

        leftCount = 0;
        rightCount = 0;

        for ( int j = start; j < end; j++ )
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
        if ( expectationCost < expectationMin - 1e-8)
        {
            expectationDivide = divide + 1;
            expectationMin = expectationCost;
        }
    }

    leftBox.Make_Empty();
    rightBox.Make_Empty();

    if ( ( expectationDivide == 0x3f3f3f3f || expectationDivide == start || expectationDivide == end ) && start < end )
        expectationDivide = ( start + end ) / 2;

    for ( int i = start; i < end; i++ )
    {
        if ( i < expectationDivide )
            leftBox = leftBox.Union( entries[i].box );
        else
            rightBox = rightBox.Union( entries[i].box );
    }
    unsigned int parentIndex = tree.size() - 1;
    Compute_Box( start, expectationDivide, leftBox);
    unsigned int rightChildIndex = tree.size();
    Compute_Box( expectationDivide, end, rightBox);
    rightChildOffset[parentIndex] = rightChildIndex;
}

// Populate tree from entries.
void Hierarchy::Build_Tree()
{
    if ( entries.empty() ) return;
    TODO;
    Box box;
    box.Make_Empty();
    for ( auto & item : entries )
    {
        box = box.Union(item.box);
    }

    Compute_Box( 0, entries.size(), box );
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
        int leafIndex = 0;
        for ( unsigned int i = 0; i <= root; i++ )
            if ( !rightChildOffset[i] ) leafIndex++;
        candidates.push_back( leafIndex - 1 );
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

