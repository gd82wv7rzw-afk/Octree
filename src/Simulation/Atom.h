#pragma once

#include <simd/simd.h>

namespace Simulation {

struct Atom 
{
    simd::float3 pos{0.f, 0.f, 0.f};
    simd::float3 vel{0.f, 0.f, 0.f};
    simd::float3 acc{0.f, 0.f, 0.f};
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Simulation ----------------------------------------------------------------------------------------------