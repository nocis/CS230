#ifndef __HIERARCHY_H__
#define __HIERARCHY_H__

#include "object.h"

/*
  A hierarchy is a binary tree.  We represent the hierarchy as a complete
  binary tree.  This allows us to represent the tree unambiguously as an
  array.  All rows (except possibly the last) are completely filled.  All
  nodes in the last row are as far to the left as possible.  The tree
  entries occur in the vector called tree in the following order.

          0
     1       2
   3   4   5   6
  7 8 9

  Note that it is possible to compute the indices for the children of a
  node from the index of the parent.  It is also possible to compute the
  index of the parent of a node from the index of a child.  Because of
  this, no pointers need to be stored.

  Note that if entries has n entries, then tree will have 2*n-1 entries.
  The last n elements of tree correspond to the elements of entries (in order).
*/

struct Entry
{
    Object* obj;
    int part;
    Box box;
};

class Hierarchy
{
public:
    // List of primitives (or parts of primitives) that can be intersected
    std::vector<Entry> entries;

    // Flattened hierarchy
    std::vector<Box> tree;

    // Reorder the entries vector so that adjacent entries tend to be nearby.
    void Reorder_Entries( unsigned int begin, unsigned int  end );

    // Populate tree from entries.
    void Build_Tree();

    // Return a list of candidates (indices into the entries list) whose
    // bounding boxes intersect the ray.
    void Intersection_Candidates(const Ray& ray, std::vector<int>& candidates, unsigned int root = 0 ) const;

    static int axis;
    double maxLength = 0.0;
    double maxPos = 0.0;
    double minPos = 0.0;
    //helper for reorder
    static bool compare_help(const Entry &a, const Entry &b);

    //Compute node bounding box
    void Compute_Box( int start, int end, Box& box );

    //Flattened hierarchy
    std::vector<unsigned int> rightChildOffset;
};
#endif
