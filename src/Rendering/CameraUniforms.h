#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
#else
    #include <simd/simd.h>
#endif

namespace Rendering {

struct CameraUniforms
{
    simd::float4x4 view;
    simd::float4x4 projection;
};

} // namespace Renderering ---------------------------------------------------------------------------------------------