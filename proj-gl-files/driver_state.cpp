#include "driver_state.h"
#include <cstring>

driver_state::driver_state()
{
}

driver_state::~driver_state()
{
    delete [] image_color;
    delete [] image_depth;
}
// This function do interpolation between two vertices
void vertices_interpolation( driver_state& state, data_geometry& result, const data_geometry& A, const data_geometry& B, float weight, const data_geometry* in[3] )
{
    result.gl_Position = ( weight * A.gl_Position ) + ( ( 1 - weight ) * B.gl_Position );
    for ( int i = 0; i < state.floats_per_vertex; i++ )
    {
        if ( state.interp_rules[i] == interp_type::flat )
        {
            result.data[i] = in[0]->data[i];
        }
        else if ( state.interp_rules[i] == interp_type::smooth )
        {
            float aw = weight / A.gl_Position[3];
            float bw = ( 1.0 - weight ) / B.gl_Position[3];
            float w = aw + bw;

            result.data[i] = ( aw * A.data[i] + bw * B.data[i] ) / w;
        }
        else if ( state.interp_rules[i] == interp_type::noperspective )
        {
            result.data[i] = ( weight * A.data[i] ) + ( ( 1 - weight ) * B.data[i] );
            //result.data[i] = ( A.gl_Position[3] * weight / result.gl_Position[3] * A.data[i] ) + ( 1 - A.gl_Position[3] * weight / result.gl_Position[3] ) * B.data[i];
        }
    }
}

void vertices_interpolation_perspective( driver_state& state, data_geometry& result, const data_geometry& A, const data_geometry& B, float weight, const data_geometry* in[3] )
{
    result.gl_Position = ( weight * A.gl_Position ) + ( ( 1 - weight ) * B.gl_Position );
    for ( int i = 0; i < state.floats_per_vertex; i++ )
    {
        if ( state.interp_rules[i] == interp_type::flat )
        {
            result.data[i] = in[0]->data[i];
        }
        else if ( state.interp_rules[i] == interp_type::smooth )
        {
            result.data[i] = ( weight * A.data[i] ) + ( ( 1 - weight ) * B.data[i] );
        }
        else if ( state.interp_rules[i] == interp_type::noperspective )
        {
            result.data[i] = ( A.gl_Position[3] * weight / result.gl_Position[3] * A.data[i] ) + ( 1 - A.gl_Position[3] * weight / result.gl_Position[3] ) * B.data[i];
        }
    }
}

void barycentrical_interpolation( driver_state& state, data_geometry& result, const data_geometry& A, const data_geometry& B, const data_geometry& C,
                                         float a, float b, float c, const data_geometry* in[3] )
{
    result.gl_Position = ( a * A.gl_Position ) + ( b * B.gl_Position ) + ( c * C.gl_Position );
    for ( int k = 0; k < state.floats_per_vertex; k++ )
    {
        if ( state.interp_rules[k] == interp_type::flat )
        {
            result.data[k] = in[0]->data[k];
        }
        else if ( state.interp_rules[k] == interp_type::smooth )
        {
            float aw = a / A.gl_Position[3];
            float bw = b / B.gl_Position[3];
            float cw = c / C.gl_Position[3];
            float w = aw + bw + cw;

            result.data[k] = ( ( aw * A.data[k] ) + ( bw * B.data[k] ) + ( cw * C.data[k] ) ) / w;
        }
        else if ( state.interp_rules[k] == interp_type::noperspective )
        {
            float aw = a;
            float bw = b;
            float cw = c;
            float w = aw + bw + cw;

            result.data[k] = ( ( aw * A.data[k] ) + ( bw * B.data[k] ) + ( cw * C.data[k] ) ) / w;
            //result.data[k] = ( a * A.data[k] ) + ( b * B.data[k] ) + ( c * C.data[k] );
        }
    }
}


// This function should allocate and initialize the arrays that store color and
// depth.  This is not done during the constructor since the width and height
// are not known when this class is constructed.
void initialize_render(driver_state& state, int width, int height)
{
    state.image_width=width;
    state.image_height=height;
    state.image_color=0;
    state.image_depth=0;

    state.image_color = new pixel[ width * height ];
    state.image_depth = new float[ width * height ];
    //memset only for one byte
    for ( int i = 0; i < width * height; i++ )
    {
        state.image_color[i] = 0x000000ff;
        state.image_depth[i] = 1.0;
    }
    //std::cout<<"TODO: allocate and initialize state.image_color and state.image_depth."<<std::endl;
}

// This function will be called to render the data that has been stored in this class.
// Valid values of type are:
//   render_type::triangle - Each group of three vertices corresponds to a triangle.
//   render_type::indexed -  Each group of three indices in index_data corresponds
//                           to a triangle.  These numbers are indices into vertex_data.
//   render_type::fan -      The vertices are to be interpreted as a triangle fan.
//   render_type::strip -    The vertices are to be interpreted as a triangle strip.
void render(driver_state& state, render_type type)
{
    // vertexShader(projection)->clipping->PerspectiveDivide->aabb->rasterization->depth->fragmentShader
    auto* verticesList = new data_geometry[ state.num_vertices ];
    for ( int i = 0; i < state.num_vertices; i++ )
    {
        verticesList[i].data = &state.vertex_data[ i * state.floats_per_vertex ];
        state.vertex_shader( { verticesList[i].data }, verticesList[i], state.uniform_data );
    }

    switch ( type )
    {
        case render_type::invalid:
            std::cout<<"invalid render type!"<<std::endl;
            exit(0);
        case render_type::triangle:
            if ( state.num_vertices % 3 )
            {
                std::cout<<"invalid vertices number!"<<std::endl;
                exit(0);
            }

            for ( int i = 0; i < state.num_vertices; i += 3 )
            {
                const data_geometry* clipData[3] = { &verticesList[ i ], &verticesList[ i + 1 ], &verticesList[ i + 2 ] };
                for ( int k = 0; k < state.floats_per_vertex; k++ )
                {
                    if ( state.interp_rules[k] == interp_type::flat )
                    {
                        verticesList[ i + 1 ].data[k] = verticesList[i].data[k];
                        verticesList[ i + 2 ].data[k] = verticesList[i].data[k];
                    }
                }
                clip_triangle( state, clipData, 0 );
            }
            break;
        case render_type::indexed:
            for ( int i = 0; i < 3 * state.num_triangles; i += 3 )
            {
                const data_geometry* clipData[3] = { &verticesList[ state.index_data[i] ], &verticesList[ state.index_data[ i + 1 ] ], &verticesList[ state.index_data[ i + 2 ] ] };
                for ( int k = 0; k < state.floats_per_vertex; k++ )
                {
                    if ( state.interp_rules[k] == interp_type::flat )
                    {
                        verticesList[ state.index_data[ i + 1 ] ].data[k] = verticesList[ state.index_data[i] ].data[k];
                        verticesList[ state.index_data[ i + 2 ] ].data[k] = verticesList[ state.index_data[i] ].data[k];
                    }
                }
                clip_triangle( state, clipData, 0 );
            }
            break;
        case render_type::fan:
            for ( int i = 1; i < state.num_vertices - 1; i++ )
            {
                const data_geometry* clipData[3] = { &verticesList[0], &verticesList[ i ], &verticesList[ i + 1 ] };
                for ( int k = 0; k < state.floats_per_vertex; k++ )
                {
                    if ( state.interp_rules[k] == interp_type::flat )
                    {
                        verticesList[ i ].data[k] = verticesList[0].data[k];
                        verticesList[ i + 1 ].data[k] = verticesList[0].data[k];
                    }
                }
                clip_triangle( state, clipData, 0 );
            }
            break;
        case render_type::strip:
            for ( int i = 0; i < state.num_vertices - 2; i++ )
            {
                const data_geometry* clipData[3] = { &verticesList[i], &verticesList[ i + 1 ], &verticesList[ i + 2 ] };
                for ( int k = 0; k < state.floats_per_vertex; k++ )
                {
                    if ( state.interp_rules[k] == interp_type::flat )
                    {
                        verticesList[ i + 1 ].data[k] = verticesList[i].data[k];
                        verticesList[ i + 2 ].data[k] = verticesList[i].data[k];
                    }
                }
                clip_triangle( state, clipData, 0 );
            }
            break;
    }

    //std::cout<<"TODO: implement rendering."<<std::endl;
}


// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.
void clip_triangle_axis( driver_state& state, const data_geometry* in[3], int axis, float faceValue, int faceIdx, data_geometry& P, data_geometry& Q )
{
    data_geometry A = *in[0];
    data_geometry B = *in[1];
    data_geometry C = *in[2];

    int idx = 0;
    int idx_V[3] = { -1, -1, -1 };

    float weightP;
    float weightQ;

    for ( int i = 0; i < 3; i++ )
    {
        if ( std::abs( in[i]->gl_Position[axis] / in[i]->gl_Position[3] ) - 1e-8 > std::abs( faceValue ) && in[i]->gl_Position[axis] / in[i]->gl_Position[3] * faceValue > 0 )
        {
            idx_V[i] = idx;
            idx++;
        }
    }

    switch ( idx )
    {
        case 0:
            clip_triangle( state, in, faceIdx + 1 );
            return;
        case 1:
        {
            for ( int i = 0; i < 3; i++ )
            {
                if ( idx_V[i] > -1 )
                {
                    //reorder i to first
                    for ( int j = 0; j < i; j++ )
                    {
                        data_geometry tmp = A;
                        A = B;
                        B = C;
                        C = tmp;
                    }
                }
            }
            //A outside of the panel

            //calculate plane distance for A, C and B respectively in CVV
            float disA = faceValue * A.gl_Position[3] - A.gl_Position[axis];
            float disC = C.gl_Position[axis] - faceValue * C.gl_Position[3];
            float disB = B.gl_Position[axis] - faceValue * B.gl_Position[3];

            weightP = std::abs( disA ) / std::abs( disA + disC );
            weightQ = std::abs( disA ) / std::abs( disA + disB );

            vertices_interpolation_perspective( state, P, C, A, weightP, in );
            vertices_interpolation_perspective( state, Q, B, A, weightQ, in );

            const data_geometry* newTriangleA[3] = { &P, &B, &C };
            const data_geometry* newTriangleB[3] = { &P, &Q, &B };

            clip_triangle( state, newTriangleA, faceIdx + 1 );
            clip_triangle( state, newTriangleB, faceIdx + 1 );
            break;
        }
        case 2:
        {
            for ( int i = 0; i < 3; i++ )
            {
                if ( idx_V[i] == -1 )
                {
                    //reorder i to first
                    for ( int j = 0; j < i; j++ )
                    {
                        data_geometry tmp = A;
                        A = B;
                        B = C;
                        C = tmp;
                    }
                }
            }
            //A inside of the panel

            //calculate plane distance for A, C and B respectively in CVV
            float disA = faceValue * A.gl_Position[3] - A.gl_Position[axis];
            float disC = C.gl_Position[axis] - faceValue * C.gl_Position[3];
            float disB = B.gl_Position[axis] - faceValue * B.gl_Position[3];

            weightP = std::abs( disB ) / std::abs( disA + disB );
            weightQ = std::abs( disC ) / std::abs( disA + disC );

            vertices_interpolation_perspective( state, P, A, B, weightP, in );
            vertices_interpolation_perspective( state, Q, A, C, weightQ, in );

            const data_geometry* newTriangleA[3] = { &A, &P, &Q };

            clip_triangle( state, newTriangleA, faceIdx + 1 );
            break;
        }
        case 3:
            return;
    }
}
void clip_triangle( driver_state& state, const data_geometry* in[3],int face )
{

    data_geometry P;
    data_geometry Q;

    P.data = new float[state.floats_per_vertex];
    Q.data = new float[state.floats_per_vertex];

    switch ( face )
    {
        case 0:
            //near
            clip_triangle_axis( state, in, 2, -1, face, P, Q );
            break;
        case 1:
            //far
            clip_triangle_axis( state, in, 2, 1, face, P, Q );
            break;
        case 2:
            //left
            clip_triangle_axis( state, in, 0, -1, face, P, Q );
            break;
        case 3:
            //right
            clip_triangle_axis( state, in, 0, 1, face, P, Q );
            break;
        case 4:
            //bottom
            clip_triangle_axis( state, in, 1, -1, face, P, Q );
            break;
        case 5:
            //top
            clip_triangle_axis( state, in, 1, 1, face, P, Q );
            break;
        default:
            data_geometry A = *in[0];
            data_geometry B = *in[1];
            data_geometry C = *in[2];
            //do perspective divide after clipping to maintain the linear interpolation during clipping
            A.gl_Position[0] = A.gl_Position[0] / A.gl_Position[3];
            A.gl_Position[1] = A.gl_Position[1] / A.gl_Position[3];
            A.gl_Position[2] = A.gl_Position[2] / A.gl_Position[3];

            B.gl_Position[0] = B.gl_Position[0] / B.gl_Position[3];
            B.gl_Position[1] = B.gl_Position[1] / B.gl_Position[3];
            B.gl_Position[2] = B.gl_Position[2] / B.gl_Position[3];

            C.gl_Position[0] = C.gl_Position[0] / C.gl_Position[3];
            C.gl_Position[1] = C.gl_Position[1] / C.gl_Position[3];
            C.gl_Position[2] = C.gl_Position[2] / C.gl_Position[3];

            const data_geometry* triangle[3] = { &A, &B, &C };
            rasterize_triangle(state, triangle);
            delete [] P.data;
            delete [] Q.data;
            return;
    }
    //std::cout<<"TODO: implement clipping. (The current code passes the triangle through without clipping them.)"<<std::endl;
    //clip_triangle(state,in,face+1);
}

// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
void rasterize_triangle(driver_state& state, const data_geometry* in[3])
{
    data_geometry A = *in[0];
    data_geometry B = *in[1];
    data_geometry C = *in[2];

    //triangle divide, make sure in[0], in[1] have same y coord
    float maxY = std::fmax( A.gl_Position[1], std::fmax( B.gl_Position[1], C.gl_Position[1] ) );
    float minY = std::fmin( A.gl_Position[1], std::fmin( B.gl_Position[1], C.gl_Position[1] ) );
    if ( A.gl_Position[1] != B.gl_Position[1] && B.gl_Position[1] != C.gl_Position[1] && C.gl_Position[1] != A.gl_Position[1] )
    {
        if ( A.gl_Position[1] > minY && A.gl_Position[1] < maxY )
        {
            float weight = ( A.gl_Position[1] - C.gl_Position[1] ) / ( B.gl_Position[1] - C.gl_Position[1] );
            data_geometry P;
            P.data = new float[state.floats_per_vertex];
            vertices_interpolation( state, P, B, C, weight, in );
            P.gl_Position[1] = A.gl_Position[1];

            const data_geometry* newTriangleA[3] = { &P, &A, &B };
            const data_geometry* newTriangleB[3] = { &A, &P, &C };
            rasterize_triangle( state, newTriangleA );
            rasterize_triangle( state, newTriangleB );
            delete [] P.data;
            return;
        }
        else if ( B.gl_Position[1] > minY && B.gl_Position[1] < maxY )
        {
            float weight = ( B.gl_Position[1] - A.gl_Position[1] ) / ( C.gl_Position[1] - A.gl_Position[1] );
            data_geometry P;
            P.data = new float[state.floats_per_vertex];
            vertices_interpolation( state, P, C, A, weight, in );
            P.gl_Position[1] = B.gl_Position[1];

            const data_geometry* newTriangleA[3] = { &P, &B, &C };
            const data_geometry* newTriangleB[3] = { &B, &P, &A };
            rasterize_triangle( state, newTriangleA );
            rasterize_triangle( state, newTriangleB );
            delete [] P.data;
            return;
        }
        else if ( C.gl_Position[1] > minY && C.gl_Position[1] < maxY )
        {
            float weight = ( C.gl_Position[1] - B.gl_Position[1] ) / ( A.gl_Position[1] - B.gl_Position[1] );
            data_geometry P;
            P.data = new float[state.floats_per_vertex];
            vertices_interpolation( state, P, A, B, weight, in );
            P.gl_Position[1] = C.gl_Position[1];

            const data_geometry* newTriangleA[3] = { &P, &C, &A };
            const data_geometry* newTriangleB[3] = { &C, &P, &B };
            rasterize_triangle( state, newTriangleA );
            rasterize_triangle( state, newTriangleB );
            delete [] P.data;
            return;
        }
    }
    else
    {
        if ( B.gl_Position[1] == C.gl_Position[1] )
        {
            data_geometry tmp = A;
            A = B;
            B = C;
            C = tmp;
        }
        else if( A.gl_Position[1] == C.gl_Position[1] )
        {
            data_geometry tmp = B;
            B = A;
            A = C;
            C = tmp;
        }
    }

    // calculate pixel position
    double A_x = ( ( A.gl_Position[0] + 1.0 )* state.image_width - 1 )  / 2.0;
    double A_y = ( ( A.gl_Position[1] + 1.0 )* state.image_height - 1 ) / 2.0;

    double B_x = ( ( B.gl_Position[0] + 1.0 )* state.image_width - 1 )  / 2.0;
    double B_y = ( ( B.gl_Position[1] + 1.0 )* state.image_height - 1 ) / 2.0;

    double C_x = ( ( C.gl_Position[0] + 1.0 )* state.image_width - 1 )  / 2.0;
    double C_y = ( ( C.gl_Position[1] + 1.0 )* state.image_height - 1 ) / 2.0;


    //if not counterclockwise, reorder this triangle!!!
    if ( ( A_x * B_y + B_x * C_y + C_x * A_y - A_x * C_y - B_x * A_y - C_x * B_y ) < 0 )
    {
        double temp = A_x;
        A_x = B_x;
        B_x = temp;

        temp = A_y;
        A_y = B_y;
        B_y = temp;

        data_geometry tmp = A;
        A = B;
        B = tmp;
    }

    if ( A_y != B_y )
    {
        std::cout<<"invalid y coord!"<<std::endl;
        exit(0);
    }

    // clipping aims to reduce the number of triangles which are going to be passed into rasterize_triangle stage
    // we can only do clipping with far and near face but simply discard those who are completely out of side faces
    // and using AABB box to narrow down the range of fragments

    double triWidth = std::abs( floor( A_x ) - ceil( B_x ) );
    double triHeight = std::abs( floor( A_y ) - ceil( C_y ) );
    int strideX = ( C_y - A_y ) > 0 ? 1 : -1;
    int strideY = ( B_x - A_x ) > 0 ? 1 : -1;

    //if ( strideX > 0)
        //return;

    double totalArea = A_x * B_y + B_x * C_y + C_x * A_y - A_x * C_y - B_x * A_y - C_x * B_y;

    double daX = ( B_y - C_y ) / totalArea;
    double daY = ( C_x - B_x ) / totalArea;

    double dbX = ( C_y - A_y ) / totalArea;
    double dbY = ( A_x - C_x ) / totalArea;

    double dcX = 0;
    double dcY = ( B_x - A_x ) / totalArea;

    double P_x = floor( A_x + 0.5 );
    double P_y = floor( A_y + 0.5 );
    // cannot use left and bottom!

    double kAC_x = ( C_x - A_x ) / ( C_y - A_y );
    double kBC_x = ( C_x - B_x ) / ( C_y - B_y );

    //enlarge range up 2 pixel
    // y offset + x offset
    triWidth -= ceil( kBC_x - kAC_x ) * 2 - 2 * ceil( std::abs( kAC_x ) ) - 2 * ceil( std::abs( kBC_x ) ) - 2;
    triHeight += 2;
    P_x -= 2 * strideX * ceil( std::abs( kAC_x ) );//get wrong without ceil, only integer or 1.x would be fine ( 1.xxxxxx is wrong )
    P_y -= 2 * strideY;

    float a = 1 + ( P_x - A_x ) * daX + ( P_y - A_y ) * daY;
    float b = 0 + ( P_x - A_x ) * dbX + ( P_y - A_y ) * dbY;
    float c = 0 + ( P_x - A_x ) * dcX + ( P_y - A_y ) * dcY;
    //start from int pos of pixel, using A_x, A_y to calculate initial barycentric coord

    for ( int i = 0; i <= triHeight; i++ )
    {
        for ( int j = -ceil( std::abs( kAC_x ) ); j <= triWidth; j++ )
        {
            float aij = a + j * strideX * daX;
            float bij = b + j * strideX * dbX;
            float cij = c + j * strideX * dcX;

            int pixelPos = (int)P_y * state.image_width + (int)( P_x + j * strideX );
            //std::cout << pixelx<<std::endl;

            if ( pixelPos > 0 && pixelPos < state.image_width* state.image_height && aij >= -1e-6 && bij >= -1e-6 && cij >= -1e-6 )
            {
                float *pixelData = new float[state.floats_per_vertex];
                data_geometry pixel;
                data_output pixelInfo;
                pixel.data = pixelData;

                barycentrical_interpolation( state, pixel, A, B, C, aij, bij, cij, in );

                state.fragment_shader( { pixel.data }, pixelInfo, state.uniform_data );
                pixelInfo.output_color *= 255.0;

                //z-test
                if ( pixel.gl_Position[2] <= state.image_depth[ pixelPos ] )
                {
                    state.image_color[ pixelPos ] = make_pixel( pixelInfo.output_color[0],
                                                                pixelInfo.output_color[1],
                                                                pixelInfo.output_color[2] );
                    state.image_depth[ pixelPos ] = pixel.gl_Position[2];
                }
                delete[] pixelData;
            }
        }

        triWidth += ( kBC_x - kAC_x );
        double oldX = P_x;
        P_x += kAC_x * strideX;
        P_y += strideY;

        if ( floor( P_x ) != floor( oldX ) )
        {
            double offset = std::abs( floor( P_x ) - floor( oldX ) );
            int sign = kAC_x > 0 ? 1 : -1;
            a += sign * offset * strideX * daX;
            b += sign * offset * strideX * dbX;
            c += sign * offset * strideX * dcX;
        }
        a += strideY * daY;
        b += strideY * dbY;
        c += strideY * dcY;
    }
    //std::cout<<"TODO: implement rasterization"<<std::endl;
}
