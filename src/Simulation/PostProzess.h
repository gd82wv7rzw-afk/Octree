#pragma once

#include "MetalStuff/MetalArray.hpp"
#include "Rendering/Vertices.h"

namespace Simulation 
{

class PostProzess
{
    public:

        using Buffer = MetalArray<Rendering::SimulationVertex>;

        float eKin(Buffer* data);

    private:

};
//----------------------------------------------------------------------------------------------------------------------

} // namespace ---------------------------------------------------------------------------------------------------------