//
//  mtlm.h
//  HelloMetalCpp2
//
//  Created by Emu on 01.04.26.
//

#pragma once
#include <simd/simd.h>

namespace mtlm {

    /// @brief the identity matrix
    /// @return the identity matrix
    simd::float4x4 identity();
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Builds a translation matrix that translates by the supplied vector
    /// @param dPos the translation vector
    /// @return the translation matrix
    simd::float4x4 translation(simd::float3 dPos);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Builds a rotation matrix that rotates an xy vector about the z-axis by an
    /// @param theta (given in radians).
    /// @return the rotation matrix
    simd::double2x2 rotation(double theta);
    //------------------------------------------------------------------------------------------------------------------
    
    /// @brief Builds a rotation matrix that rotates about the supplied axis by an
    /// @param theta (given in radians).
    /// @param axis The axis should be normalized.
    /// @return the rotation matrix
    simd::float4x4 rotation(simd::float3 axis, float theta);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Builds a scale matrix that uniformly scales all axes by the supplied factor
    /// @param factor the sthe scale matrix
    simd::float4x4 scale(float factor);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Builds a symmetric perspective projection matrix with the supplied aspect ratio,
    /// @param fovy vertical field of view (in radians)
    /// @param aspect aspect ratio
    /// @param near the near clib distance
    /// @param fare the fare clib distance
    /// @return the projection matrix
    simd::float4x4 perspective_projection(float fovy, float aspect, float near, float far);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief convert double3 to float3
    inline simd::float3 to_float3(simd::double3 v)
    {
        return { float(v.x), float(v.y), float(v.z) };
    }
    //------------------------------------------------------------------------------------------------------------------

    /// @brief convert float3 to double3
    inline simd::double3 to_double3(simd::float3 v)
    {
        return { float(v.x), float(v.y), float(v.z) };
    }
    //------------------------------------------------------------------------------------------------------------------
    
    simd::float4x4 camera_view(simd::float3 right, simd::float3 up, simd::float3 forwards, simd::float3 pos);
}
//----------------------------------------------------------------------------------------------------------------------
