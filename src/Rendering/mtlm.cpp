//
//  mtlm.cpp
//  HelloMetalCpp2
//
//  Created by Emu on 01.04.26.
//

#include "mtlm.h"

#include <math.h>
#include <simd/simd.h>
#include <simd/vector_make.h>

simd::float4x4 mtlm::identity() {
    simd_float4 col0 = {1.0f, 0.0f, 0.0f, 0.0f};
    simd_float4 col1 = {0.0f, 1.0f, 0.0f, 0.0f};
    simd_float4 col2 = {0.0f, 0.0f, 1.0f, 0.0f};
    simd_float4 col3 = {0.0f, 0.0f, 0.0f, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}
//----------------------------------------------------------------------------------------------------------------------

simd::float4x4 mtlm::translation(simd::float3 dPos) {
    simd_float4 col0 = {1.0f, 0.0f, 0.0f, 0.0f};
    simd_float4 col1 = {0.0f, 1.0f, 0.0f, 0.0f};
    simd_float4 col2 = {0.0f, 0.0f, 1.0f, 0.0f};
    simd_float4 col3 = {dPos[0], dPos[1], dPos[2], 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}
//----------------------------------------------------------------------------------------------------------------------

simd::double2x2 mtlm::rotation(double theta) {
    double c = cosf(theta);
    double s = sinf(theta);
    simd::double2 col0 = simd_make_double2(c,s);
    simd::double2 col1 = simd_make_double2(-s,c);
    return simd_matrix(col0, col1);
}
//----------------------------------------------------------------------------------------------------------------------

simd::float4x4 mtlm::rotation(simd::float3 axis, float theta) {
    float c = cosf(theta);
    float s = sinf(theta);
    
    simd::float4 col0 = {
        float(axis[0] * axis[0] * (1.0 - c) + c),
        float(axis[0] * axis[1] * (1.0 - c) + axis[2] * s),
        float(axis[0] * axis[2] * (1.0 - c) - axis[1] * s),
        0.0f
    };

    simd::float4 col1 = {
        float(axis[1] * axis[0] * (1.0 - c) - axis[2] * s),
        float(axis[1] * axis[1] * (1.0 - c) + c),
        float(axis[1] * axis[2] * (1.0 - c) + axis[0] * s),
        0.0f
    };

    simd::float4 col2 = {
        float(axis[2] * axis[0] * (1.0 - c) + axis[1] * s),
        float(axis[2] * axis[1] * (1.0 - c) - axis[0] * s),
        float(axis[2] * axis[2] * (1.0 - c) + c),
        0.0f
    };

    simd::float4 col3 = {0.0f, 0.0f, 0.0f, 1.0f};

    return simd::float4x4(col0, col1, col2, col3);
}
//----------------------------------------------------------------------------------------------------------------------

simd::float4x4 mtlm::scale(float factor) {
    simd_float4 col0 = {factor,   0.0f,   0.0f, 0.0f};
    simd_float4 col1 = {  0.0f, factor,   0.0f, 0.0f};
    simd_float4 col2 = {  0.0f,   0.0f, factor, 0.0f};
    simd_float4 col3 = {  0.0f,   0.0f,   0.0f, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}
//----------------------------------------------------------------------------------------------------------------------

simd::float4x4 mtlm::perspective_projection(float fovy, float aspect, float near, float far) {

    float yScale = 1.0 / tanf(fovy * 0.5);
    float xScale = yScale / aspect;
    float zRange = near - far;
    float zScale = (far) / zRange;

    simd_float4 col0 = { xScale,   0.0f,    0.0f,  0.0f};
    simd_float4 col1 = {   0.0f, yScale,    0.0f,  0.0f};
    simd_float4 col2 = {   0.0f,   0.0f,  zScale, -1.0f};
    simd_float4 col3 = {   0.0f,   0.0f, near*zScale,  0.0f};
    return simd_matrix(col0, col1, col2, col3);
}
//----------------------------------------------------------------------------------------------------------------------

simd::float4x4 mtlm::camera_view(simd::float3 right, simd::float3 up, simd::float3 forwards, simd::float3 pos) {


    simd_float4 col0 = {right[0], up[0], -forwards[0], 0.0f};
    simd_float4 col1 = {right[1], up[1], -forwards[1], 0.0f};
    simd_float4 col2 = {right[2], up[2], -forwards[2], 0.0f};
    simd_float4 col3 = {-simd::dot(right, pos), -simd::dot(up, pos), simd::dot(forwards, pos), 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}


