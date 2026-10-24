#pragma once

#include <simd/simd.h>

namespace Simulation {

struct Branch 
{
    simd::float3 com{0.f, 0.f, 0.f};
    float width{1.f};
    int order{1};
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Simulation ----------------------------------------------------------------------------------------------