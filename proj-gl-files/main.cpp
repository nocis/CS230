/**
 * main.cpp
 * -------------------------------
 * This is simple testbed for your GLSL implementation.
 *
 * Usage: ./driver -i <input-file> [ -s <solution-file> ] [ -o <stats-file> ]
 *     <input-file>      File with commands to run
 *     <solution-file>   File with solution to compare with
 *     <stats-file>      Dump statistics to this file rather than stdout
 *
 * Only the -i is manditory.  You must specify a test to run.  For example:
 *
 * ./driver -i 00.txt
 *
 * This will save the result to output.png.  You may compare this result with a
 * reference solution:
 *
 * ./driver -i 00.txt -s 00.png
 *
 * This will compute and output a measure of the difference ("diff: 0.23"); to
 * pass a test this error must be below the test's threshold.  An image is
 * output to diff.png which visually shows where the differences are in the
 * results, which can help you track down any differences.
 *
 * The -o flag is used for the grading script, so that grading will not be
 * confused by debug print statements.
 */
#include <cassert>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>
#include <chrono>
#include "driver_state.h"
#include <unistd.h>

void parse(const char* test_file, driver_state& state);
void dump_png(pixel* data,int width,int height,const char* filename);
void read_png(pixel*& data,int& width,int& height,const char* filename);

// Compare the computed solution (in state) to the solution_file
void compare(driver_state& state, FILE* stats_file, const char* solution_file)
{
    // Allocate and initialize space for the solution and difference images
    int width_sol = 0;
    int height_sol = 0;
    int size = state.image_width*state.image_height;
    pixel* image_diff = new pixel[size];
    pixel* image_sol = new pixel[size];
    pixel black = make_pixel(0,0,0);
    for(int i=0;i<size;i++)
    {
        image_sol[i]=black;
        image_diff[i]=black;
    }

    // Read the solution file, do some sanity checks.
    read_png(image_sol, width_sol, height_sol, solution_file);
    if(state.image_width!=width_sol || state.image_height!=height_sol)
    {
        std::cerr<<"Solution dimensions ("<<width_sol<<","<<height_sol
                 <<") do not match problem size ("
                 <<state.image_width<<","<<state.image_height<<")"<<std::endl;
        delete [] image_diff;
        delete [] image_sol;
        exit(EXIT_FAILURE);
    }

    // Compare the computed an solution images pixel by pixel
    int total_diff = 0;
    for(int i=0;i<size;i++)
    {
        int A=image_sol[i];
        int B=state.image_color[i];
        int rA,gA,bA,rB,gB,bB;
        from_pixel(A,rA,gA,bA);
        from_pixel(B,rB,gB,bB);
        int r=abs(rA-rB);
        int g=abs(gA-gB);
        int b=abs(bA-bB);
        int diff=std::max(std::max(r,g),b);
        total_diff += diff;
        pixel diff_color=make_pixel(diff, diff, diff);
        image_diff[i]=diff_color;
    }

    // Dump out the difference so we can see visually what is different
    dump_png(image_diff,state.image_width,state.image_height,"diff.png");

    // Dump out stats so we can determine whether we are close enough
    fprintf(stats_file, "diff: %.2f\n",total_diff/(2.55*size));
    delete [] image_diff;
    delete [] image_sol;
}

// Provide assistance in calling this program
void Usage(const char* prog_name)
{
    std::cerr<<"Usage: "<<prog_name<<" -i <input-file> [ -s <solution-file> ] [ -o <stats-file> ]"<<std::endl;
    std::cerr<<"    <input-file>      File with commands to run"<<std::endl;
    std::cerr<<"    <solution-file>   File with solution to compare with"<<std::endl;
    std::cerr<<"    <stats-file>      Dump statistics to this file rather than stdout"<<std::endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    const char* solution_file = 0;
    const char* input_file = 0;
    const char* statistics_file = 0;
    
    driver_state state;

    // Parse commandline options
    while(1)
    {
        int opt = getopt(argc, argv, "s:i:o:");
        if(opt==-1) break;
        switch(opt)
        {
            case 's': solution_file = optarg; break;
            case 'i': input_file = optarg; break;
            case 'o': statistics_file = optarg; break;
        }
    }

    // Sanity checks
    if(!input_file)
    {
        std::cerr<<"Test file required.  Use -i."<<std::endl;
        Usage(argv[0]);
    }

    // Parse the input file, setup state, request renders
    parse(input_file, state);

    FILE* stats_file = stdout;
    if(statistics_file) stats_file = fopen(statistics_file, "w");

    // Compare computed solution to solution file, if provided
    if(solution_file)
        compare(state, stats_file, solution_file);

    // Save the computed solution to file
    dump_png(state.image_color,state.image_width,state.image_height,"output.png");

    if(stats_file != stdout) fclose(stats_file);
    return 0;
}
