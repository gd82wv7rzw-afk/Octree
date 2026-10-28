#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
#else
    #include <simd/simd.h>
#endif

namespace Simulation {

struct Atom
{
#ifdef __METAL_VERSION__
    float3 pos{0.f, 0.f, 0.f};
    float3 vel{0.f, 0.f, 0.f};
    float3 acc{0.f, 0.f, 0.f};
#else
    simd::float3 pos{0.f, 0.f, 0.f};
    simd::float3 vel{0.f, 0.f, 0.f};
    simd::float3 acc{0.f, 0.f, 0.f};
#endif
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Simulation ----------------------------------------------------------------------------------------------