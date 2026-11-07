#pragma once

#include "MetalStuff/MetalArray.hpp"
#include "Rendering/Vertices.h"

#include <cstdint>

namespace Rendering
{

class TripleBuffer
{
    public:

        using Buffer = MetalArray<SimulationVertex>;
        using BufferIndex = uint8_t;

        struct BufferRoles
        {
            BufferIndex nextWrite;
            BufferIndex nextOutput;
            BufferIndex currentOutput;
        };
        //--------------------------------------------------------------------------------------------------------------
 
        TripleBuffer() : mRoles(encode({0, 1, 2})) {}
        //--------------------------------------------------------------------------------------------------------------
 
        void allocate(MTL::Device* device, size_t count);
        //--------------------------------------------------------------------------------------------------------------
 
        Buffer* getInputBuffer();   // Simulator
        Buffer* getOutputBuffer();  // Renderer
        //--------------------------------------------------------------------------------------------------------------
 
        void publishInput();        // Simulator: Kopieren abgeschlossen
        void releaseOutput();       // Renderer: GPU-Rendering abgeschlossen
        //--------------------------------------------------------------------------------------------------------------

    private:

        static uint32_t encode(const BufferRoles& roles);
        static BufferRoles decode(uint32_t value);
        //--------------------------------------------------------------------------------------------------------------

        Buffer test;

        Buffer mBuffers[3];
        uint32_t mRoles;
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Rendering -----------------------------------------------------------------------------------------------