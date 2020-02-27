#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <sstream>
#include <vector>
#include "driver_state.h"
#include "shaders.h"

// Parse the input file and issue commands
void parse(const char* test_file, driver_state& state)
{
    // Open file, make sure this succeeded
    FILE* F = fopen(test_file,"r");
    if(!F)
    {
        printf("Failed to open file '%s'\n",test_file);
        exit(EXIT_FAILURE);
    }

    // Initialize the maps that allow us to access shaders by name.
    register_named_shaders();

    // scratch space for parsing
    char buff[1000];
    ivec3 e;

    // Local copies of the data that will eventually be stored in the driver for
    // rending.  data => driver.vertex_data, indices => driver.index_data,
    // uniform => driver.uniform_data.  Note that the driver only stores
    // pointers into these std::vector's.  This is normally a very bad idea,
    // since those pointers may change if the std::vectors are modified.  We
    // must be careful to set the driver pointers only immediately before
    // issuing the rendering commands.  Since the renders occur within this
    // function, this data will not be used after these have gone out of scope.
    int floats_per_vertex=0;
    std::vector<float> data;
    std::vector<ivec3> indices;
    std::vector<float> uniform;

    // Parse the input, line by line
    while(fgets(buff, sizeof(buff), F))
    {
        std::stringstream ss(buff);
        std::string item,name;

        // If we did not get a line, the line is empty, or the line is a
        // comment, then move on.
        if(!(ss>>item) || !item.size() || item[0]=='#') continue;
        if(item=="size")
        {
            // format: size <w> <h>
            // Set image size.
            int w,h;
            ss>>w>>h;
            initialize_render(state, w, h);
        }
        else if(item=="vertex_data")
        {
            // format: vertex_data <flags>
            // The flags consists of a string of the characters f, n, or s.

            // There are floats_per_vertex characters in the string.  The
            // character indicates how the corresponding float should be
            // interpolated to pixels (fragments) within a triangle.  The options are:
            // f: flat; use the data from the first vertex of the triangle
            // n: non-perspective-correct interpolation
            // s: smooth; perspective-correct interpolation
            // The length of the string is used to deduce floats_per_vertex.
            ss>>buff;
            int i;
            for(i=0;buff[i];i++)
            {
                if(buff[i]=='s') state.interp_rules[i]=interp_type::smooth;
                else if(buff[i]=='n') state.interp_rules[i]=interp_type::noperspective;
                else if(buff[i]=='f') state.interp_rules[i]=interp_type::flat;
                else assert("invalid interpolation type" && 0);
            }
            floats_per_vertex=i;
        }
        else if(item=="v")
        {
            // format: v <float> <float> <float> ...
            // Provides the per-vertex data for one vertex
            // There should be floats_per_vertex floats on the line.
            float x;
            for(int i=0;i<floats_per_vertex;i++)
            {
                if(ss>>x) data.push_back(x);
                else data.push_back(0);
            }
        }
        else if(item=="f")
        {
            // format: f <index> <index> <index>
            // Provides the indices of the vertices for one triangle.
            ss>>e;
            indices.push_back(e);
        }
        else if(item=="render")
        {
            // format: render <type>
            // Render the information that has been accumulated, and then clear
            // out the state for the next render.  The accumulated data is to be
            // interpreted according to <type>, which may be:
            // triangle - Each group of three vertices corresponds to a triangle.
            // indexed -  Each group of three indices in index_data corresponds
            //            to a triangle.  These numbers are indices into vertex_data.
            // fan -      The vertices are to be interpreted as a triangle fan.
            // strip -    The vertices are to be interpreted as a triangle strip.
            // Assign pointers in driver immediately before doing the render to
            // avoid memory errors.
            ss>>name;
            state.vertex_data=&data[0];
            state.num_vertices=data.size()/floats_per_vertex;
            state.floats_per_vertex=floats_per_vertex;
            state.index_data=indices.size()?&indices[0][0]:0;
            state.num_triangles=indices.size();
            state.uniform_data=uniform.size()?&uniform[0]:0;
            render_type t;
            if(name=="indexed") t=render_type::indexed;
            else if(name=="fan") t=render_type::fan;
            else if(name=="triangle") t=render_type::triangle;
            else if(name=="strip") t=render_type::strip;
            else assert("invalid render type" && 0);
            render(state,t);
            data.clear();
            indices.clear();
        }
        else if(item=="uniform")
        {
            // format: uniform <float> <float> <float> ...
            // Provide all of the uniform data for the render.
            uniform.clear();
            float x;
            while(ss>>x) uniform.push_back(x);
        }
        else if(item=="vertex_shader")
        {
            // format: vertex_shader <name>
            // Set the vertex shader
            ss>>name;
            state.vertex_shader=vertex_shader_map[name];
            assert(state.vertex_shader);
        }
        else if(item=="fragment_shader")
        {
            // format: fragment_shader <name>
            // Set the fragment shader
            ss>>name;
            state.fragment_shader=fragment_shader_map[name];
            assert(state.fragment_shader);
        }
        else
        {
            // Check for parse errors.
            int len=strlen(buff);
            if(buff[len-1]=='\n') buff[len-1]=0;
            printf("Unrecognized command: '%s'\n",buff);
            exit(EXIT_FAILURE);
        }
    }
    fclose(F);
}
