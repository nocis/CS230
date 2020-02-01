#include <map>
#include <sstream>
#include <string>
#include <png.h>
#include "direction_light.h"
#include "flat_shader.h"
#include "mesh.h"
#include "phong_shader.h"
#include "plane.h"
#include "point_light.h"
#include "reflective_shader.h"
#include "render_world.h"
#include "sphere.h"
#include "spot_light.h"
void Read_png_local(Pixel*& data,int& width,int& height,const char* filename);
void Parse(Render_World& world,int& width,int& height,const char* test_file)
{
    FILE* F = fopen(test_file,"r");
    if(!F)
    {
        std::cout<<"Failed to open file '"<<test_file<<"'\n";
        exit(EXIT_FAILURE);
    }

    double f0;
    char buff[1000];
    vec3 u,v,w;
    std::string name,s0,s1,s2;
    
    std::map<std::string,vec3> colors;
    std::map<std::string,Object*> objects;
    std::map<std::string,Shader*> shaders;
    std::map<std::string, unsigned int> textures;

    world.enable_backface = true;
    while(fgets(buff, sizeof(buff), F))
    {
        std::stringstream ss(buff);
        std::string item,name;
        if(!(ss>>item) || !item.size() || item[0]=='#') continue;
        if(item=="size")
        {
            ss>>width>>height;
            assert(ss);
        }
        else if(item=="color")
        {
            ss>>name>>u;
            assert(ss);
            colors[name]=u;
        }
        else if(item=="plane")
        {
            ss>>name>>u>>v>>s0;
            assert(ss);
            Object* o=new Plane(u,v);
            std::map<std::string,Shader*>::const_iterator sh=shaders.find(s0);
            assert(sh!=shaders.end());
            o->material_shader=sh->second;
            if(name=="-") world.objects.push_back(o);
            else objects[name]=o;
        }
        else if(item=="sphere")
        {
            ss>>name>>u>>f0>>s0;
            //assert(ss);//modified for texture
            Object* o=new Sphere(u,f0);
            std::map<std::string,Shader*>::const_iterator sh=shaders.find(s0);
            assert(sh!=shaders.end());
            o->material_shader=sh->second;
            if(name=="-") world.objects.push_back(o);
            else objects[name]=o;

            ss>>s2;
            if (  ss && !s2.empty() && s2 != "-" )
            {
                std::map<std::string, unsigned int>::const_iterator tx=textures.find(s2);
                assert(tx!=textures.end());
                o->texture_id=tx->second;
            }
        }
        else if(item=="mesh")
        {
            ss>>name>>s0>>s1;
            //assert(ss); //modified for texture
            Mesh* o=new Mesh;
            o->Read_Obj(s0.c_str());
            std::map<std::string,Shader*>::const_iterator sh=shaders.find(s1);
            assert(sh!=shaders.end());
            o->material_shader=sh->second;
            if(name=="-") world.objects.push_back(o);
            else objects[name]=o;

            ss>>s2;
            if ( ss && !s2.empty() && s2 != "-" )
            {
                std::map<std::string, unsigned int>::const_iterator tx=textures.find(s2);
                assert(tx!=textures.end());
                o->texture_id=tx->second;
            }
        }
        else if(item=="flat_shader")
        {
            ss>>name>>s0;
            assert(ss);
            std::map<std::string,vec3>::const_iterator c0=colors.find(s0);
            assert(c0!=colors.end());
            shaders[name]=new Flat_Shader(world,c0->second);
            world.shaders.push_back( shaders[name] );
        }
        else if(item=="phong_shader")
        {
            ss>>name>>s0>>s1>>s2>>f0;
            assert(ss);
            std::map<std::string,vec3>::const_iterator c0=colors.find(s0);
            std::map<std::string,vec3>::const_iterator c1=colors.find(s1);
            std::map<std::string,vec3>::const_iterator c2=colors.find(s2);
            assert(c0!=colors.end());
            assert(c1!=colors.end());
            assert(c2!=colors.end());
            shaders[name]=new Phong_Shader(world,c0->second,c1->second,c2->second,f0);
            world.shaders.push_back( shaders[name] );
        }
        else if(item=="reflective_shader")
        {
            world.enable_backface = false;
            ss>>name>>s0>>f0;
            assert(ss);
            std::map<std::string,Shader*>::const_iterator sh=shaders.find(s0);
            assert(sh!=shaders.end());
            shaders[name]=new Reflective_Shader(world,sh->second,f0);
            world.shaders.push_back( shaders[name] );
        }
        else if(item=="point_light")
        {
            ss>>u>>s0>>f0;
            assert(ss);
            std::map<std::string,vec3>::const_iterator c0=colors.find(s0);
            assert(c0!=colors.end());
            world.lights.push_back(new Point_Light(u,c0->second,f0));
        }
        else if(item=="direction_light")
        {
            ss>>u>>s0>>f0;
            assert(ss);
            std::map<std::string,vec3>::const_iterator c0=colors.find(s0);
            assert(c0!=colors.end());
            world.lights.push_back(new Direction_Light(u,c0->second,f0));
        }
        else if(item=="spot_light")
        {
            double max_angle,exponent;
            vec3 direction;
            ss>>u>>s0>>f0>>max_angle>>exponent>>direction;
            assert(ss);
            std::map<std::string,vec3>::const_iterator c0=colors.find(s0);
            assert(c0!=colors.end());
            world.lights.push_back(new Spot_Light(u,c0->second,f0,max_angle,exponent,direction));
        }
        else if(item=="ambient_light")
        {
            ss>>s0>>f0;
            assert(ss);
            std::map<std::string,vec3>::const_iterator c0=colors.find(s0);
            assert(c0!=colors.end());
            world.ambient_color=c0->second;
            world.ambient_intensity=f0;
        }
        else if(item=="camera")
        {
            ss>>u>>v>>w>>f0;
            assert(ss);
            world.camera.Position_And_Aim_Camera(u,v,w);
            world.camera.Focus_Camera(1,(double)width/height,f0*(pi/180));
        }
        else if(item=="background")
        {
            ss>>s0;
            assert(ss);
            std::map<std::string,Shader*>::const_iterator sh=shaders.find(s0);
            assert(sh!=shaders.end());
            world.background_shader=sh->second;
        }
        else if(item=="enable_shadows")
        {
            ss>>world.enable_shadows;
            assert(ss);
        }
        else if(item=="recursion_depth_limit")
        {
            ss>>world.recursion_depth_limit;
            assert(ss);
        }
        else if(item=="texture")
        {
            ss>>name>>s0;
            assert(ss);
            int width2 = 0, height2 = 0;
            Pixel* data_sol = 0;
            Read_png_local(data_sol,width2,height2,s0.c_str());
            if ( width2 == 1024 && height2 == 1024 )
            {
                //in case of some does not have texture
                world.textures.push_back( nullptr );
                textures[name]=world.textures.size();
                world.textures.push_back( data_sol );
            }
        }
        else if(item=="skybox")
        {
            ss>>s0;
            assert(ss);
            int width2 = 0, height2 = 0;
            Pixel* data_sol = 0;
            Read_png_local(data_sol,width2,height2,s0.c_str());
            if ( width2 == 4096 && height2 == 3072 )
            {
                world.skybox = data_sol;
            }
        }
        else
        {
            std::cout<<"Failed to parse: "<<buff<<std::endl;
            exit(EXIT_FAILURE);
        }
    }
    if(!world.background_shader)
        world.background_shader=new Flat_Shader(world,vec3());
    world.camera.Set_Resolution(ivec2(width,height));
}

void Read_png_local(Pixel*& data,int& width,int& height,const char* filename)
{
    FILE *file = fopen(filename, "rb");
    assert(file);

    unsigned char header[8];
    int num_read=fread(&header, 1, sizeof header, file);
    assert(num_read==sizeof header);
    int ret_sig=png_sig_cmp((png_bytep)header, 0, sizeof header);
    assert(!ret_sig);
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    assert(png_ptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr);
    png_infop end_info = png_create_info_struct(png_ptr);
    assert(end_info);
    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, sizeof header);
    png_read_info(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if(bit_depth == 16)
        png_set_strip_16(png_ptr);

    if(bit_depth < 8)
        png_set_packing(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
        png_set_swap_alpha(png_ptr);

    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
        png_set_bgr(png_ptr);

    if(color_type == PNG_COLOR_TYPE_RGB)
        png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

    height = png_get_image_height(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);

    data = new Pixel[width * height];

    png_read_update_info(png_ptr, info_ptr);

    for(int i = 0; i < height; i++)
        png_read_row(png_ptr, (png_bytep)(data + (height-i-1) * width), 0);

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(file);
}