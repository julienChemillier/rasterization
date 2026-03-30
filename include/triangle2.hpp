#ifndef TRIANGLE_HPP
#define TRIANGLE_HPP

#include "points.hpp"
#include <algorithm>

struct triangle2 {
    float2 a;
    float2 b;
    float2 c;
};

struct triangle3 {
    float3 a;
    float3 b;
    float3 c;
    
    // Normales pour le Gouraud Shading
    float3 na, nb, nc;
};


#endif