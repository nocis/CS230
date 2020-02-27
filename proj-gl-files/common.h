#ifndef __COMMON_H__
#define __COMMON_H__

#include "vec.h"

// This file contains common data types and definitions that are used elsewhere
// in this project.  You should not need to make any changes in this file.

// Data type for a pixel, which packs RGBA values in a 32-bit int.  Use
// make_pixel and from_pixel to pack or unpack this data.
typedef unsigned int pixel;

inline pixel make_pixel(int r, int g, int b)
{
    return (r<<24)|(g<<16)|(b<<8)|0xff;
}

inline void from_pixel(pixel p, int& r, int& g, int& b)
{
    r = (p>>24)&0xff;
    g = (p>>16)&0xff;
    b = (p>>8)&0xff;
}

// Maximum number of floats that may be stored per vertex.  This is useful since
// it allows you to allocate fixed-sized arrays for things, such as local copies
// of data for a vertex.  You are guaranteed that
// driver.floats_per_vertex<=MAX_FLOATS_PER_VERTEX.
static const int MAX_FLOATS_PER_VERTEX = 64;

class driver_state;

// This is the data that is stored for one vertex.  Although a real GLSL vertex
// shader has many built-in items (commented out), our version just has the
// custom data (pointed to by "data").  This pointer should point to the first
// float within the state.vertex_data array for the vertex being processed.
// Technically the driver cannot determine from this data what the positions
// are, though in practice they are always at the beginning.  When you start
// calling the vertex shader, the shader will populate a data_geometry
// structure, which stores the positions explicitly.  This will allow you to
// do geometry processing (clipping, rasterization) on the triangle.
struct data_vertex
{
    float * data;
    // int gl_VertexID;
    // int gl_InstanceID;
    // int gl_DrawID;
    // int gl_BaseVertex;
    // int gl_BaseInstance;
};

// This is the data that is the output from the vertex shader.  data should be
// pointed to a new array whose size is MAX_FLOATS_PER_VERTEX.  The meaning of
// data is the same as for data_vertex.  In addition, this structure has an
// extra entry gl_Position, which holds the vertex position as a homogeneous
// vector.  This is how your code learns about the positions of vertices.
struct data_geometry
{
    vec4 gl_Position;
    // float gl_PointSize;
    // float gl_ClipDistance[];

    float * data;
};

// This is the data that is the input to the fragment shader.  data should be
// pointed to a new array whose size is MAX_FLOATS_PER_VERTEX.  This data is
// interpolated from the per-vertex data.
struct data_fragment
{
    // vec4 gl_FragCoord;
    // bool gl_FrontFacing;
    // vec2 gl_PointCoord;
    // int gl_SampleID;
    // vec2 gl_SamplePosition;
    // int gl_SampleMaskIn[];
    // float gl_ClipDistance[];
    // int gl_PrimitiveID;
    // int gl_Layer;
    // int gl_ViewportIndex;

    float * data;
};

// This structure stores the color of a pixel (fragment) and is populated by the
// fragment shader.  In real GLSL shaders, this is done a bit differently, and
// shaders may output more than one color buffer.
struct data_output
{
    vec4 output_color;
    // float gl_FragDepth;
    // int gl_SampleMask[];
};

// Signatures for vertex shaders and fragment shaders.
typedef void (*shader_v)(const data_vertex&, data_geometry&,const float *);

typedef void (*shader_f)(const data_fragment&, data_output&,const float *);

// Different interpolation strategies that may be used to interpolate data from
// triangle vertices to the pixels (fragments) inside the triangle.
enum class interp_type {invalid, flat, smooth, noperspective};

// Different layouts for providing triangle data.
enum class render_type {invalid, indexed, triangle, fan, strip};

#endif
