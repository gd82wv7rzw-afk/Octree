#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
#else
    #include <simd/simd.h>
#endif

namespace Simulation {

struct Branch
{
#ifdef __METAL_VERSION__
    float3 com{0.f, 0.f, 0.f};
    float3 min{0.f, 0.f, 0.f};
#else
    simd::float3 com{0.f, 0.f, 0.f};
    simd::float3 min{0.f, 0.f, 0.f};
#endif
    float width{1.f};
    int order{1};
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Simulation ----------------------------------------------------------------------------------------------