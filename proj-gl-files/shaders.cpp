#include "shaders.h"

// Lookup maps to access a shader by name.
std::map<std::string,shader_v> vertex_shader_map;
std::map<std::string,shader_f> fragment_shader_map;

// Simplest useful vertex shader; just copies over the positions.
void vertex_shader_trivial(const data_vertex& in, data_geometry& out,
        const float * uniform_data)
{
    const vertex_p& v = *(const vertex_p*)in.data;
    out.gl_Position = vec4(v.position,1);
}

// This shader (1) transforms vertices and (2) copies over colors.
void vertex_shader_color(const data_vertex& in, data_geometry& out,
        const float * uniform_data)
{
    const vertex_pc& v = *(const vertex_pc*)in.data;
    vertex_pc& o = *(vertex_pc*)out.data;
    mat4& xform = *(mat4*)uniform_data;
    out.gl_Position = xform * vec4(v.position,1);
    o.color = v.color;
}

// This shader just transforms vertices
void vertex_shader_transform(const data_vertex& in, data_geometry& out,
        const float * uniform_data)
{
    vertex_p& v = *(vertex_p*)in.data;
    mat4& xform = *(mat4*)uniform_data;
    out.gl_Position = xform * vec4(v.position,1);
}

// Simple fragment shader: set the fragment to red
void fragment_shader_red(const data_fragment& in, data_output& out,
    const float * uniform_data)
{
    out.output_color = vec4(1,0,0,0);
}

// Simple fragment shader: set the fragment to green
void fragment_shader_green(const data_fragment& in, data_output& out,
    const float * uniform_data)
{
    out.output_color = vec4(0,1,0,0);
}

// Simple fragment shader: set the fragment to blue
void fragment_shader_blue(const data_fragment& in, data_output& out,
    const float * uniform_data)
{
    out.output_color = vec4(0,0,1,0);
}

// Simple fragment shader: set the fragment to white
void fragment_shader_white(const data_fragment& in, data_output& out,
    const float * uniform_data)
{
    out.output_color = vec4(1,1,1,0);
}

// Simple fragment shader: pass through globally constant color
void fragment_shader_uniform(const data_fragment& in, data_output& out,
    const float * uniform_data)
{
    transform_color& tc = *(transform_color*)uniform_data;
    out.output_color = vec4(tc.color,0);
}

// Simple fragment shader: pass through interpolated per-vertex color
void fragment_shader_gouraud(const data_fragment& in, data_output& out,
    const float * uniform_data)
{
    vertex_pc& v = *(vertex_pc*)in.data;
    out.output_color = vec4(v.color,0);
}

// Assign shaders to the maps so they can be accessed by name.
void register_named_shaders()
{
    vertex_shader_map["trivial"]=vertex_shader_trivial;
    vertex_shader_map["transform"]=vertex_shader_transform;
    vertex_shader_map["color"]=vertex_shader_color;
    fragment_shader_map["red"]=fragment_shader_red;
    fragment_shader_map["green"]=fragment_shader_green;
    fragment_shader_map["blue"]=fragment_shader_blue;
    fragment_shader_map["white"]=fragment_shader_white;
    fragment_shader_map["gouraud"]=fragment_shader_gouraud;
    fragment_shader_map["uniform"]=fragment_shader_uniform;
}
