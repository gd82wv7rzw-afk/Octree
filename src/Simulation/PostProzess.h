#pragma once

#include "MetalStuff/MetalArray.hpp"
#include "Rendering/Vertices.h"

#include <simd/simd.h>

namespace Simulation 
{

class PostProzess
{
    public:

        using Buffer = MetalArray<Rendering::SimulationVertex>;

        simd::float3 eKin(Buffer* data);

    private:

};
//----------------------------------------------------------------------------------------------------------------------

} // namespace ---------------------------------------------------------------------------------------------------------