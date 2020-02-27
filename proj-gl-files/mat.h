#ifndef __mat__
#define __mat__

#include <cmath>
#include <iostream>
#include <cassert>
#include "vec.h"

template<class T, int m, int n=m>
struct mat
{
    T x[n*m];

    mat()
    {make_zero();}

    T& operator()(int i,int j)
    {return x[i*n+j];}

    const T& operator()(int i,int j) const
    {return x[i*n+j];}

    void make_zero()
    {fill(0);}

    void make_id()
    {assert(m==n);fill(0);for(int i=0;i<m*n;i+=m+1) x[i]=1;}

    void fill(T value)
    {for(int i = 0; i < m*n; i++) x[i] = value;}

    vec<T,m> operator* (const vec<T,n>& u) const
    {
        vec<T,m> v;
        for(int i=0;i<m;i++)
            for(int j=0;j<n;j++)
                v[i]+=x[i*n+j]*u[j];
        return v;
    }
};

typedef mat<float,3> mat3;
typedef mat<float,4> mat4;

#endif
