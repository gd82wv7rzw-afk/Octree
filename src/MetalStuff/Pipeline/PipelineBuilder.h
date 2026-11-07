#pragma once

#include "MetalStuff/Pipeline/PipelineDescription.h"

#include <Metal/Metal.hpp>
#include <cstddef>

class PipelineBuilder {
    
    public:
    
        /// @brief Creates a PipelineBuilder instance.
        /// @details
        /// Stores a non-owning reference to the Metal device.
        /// Used for creating RenderPipelineState objects and loading shader libraries.
        /// @param device   Metal device used for pipeline creation
        PipelineBuilder(MTL::Device* device);
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Default destructor.
        /// @details
        /// No explicit cleanup required because all owned resources are managed via RAII.
        ~PipelineBuilder() = default;
        //--------------------------------------------------------------------------------------------------------------
    
        /// @brief Builds a compute pipeline state from a kernel function.
        /// @param filepath Path to the metallib.
        /// @param kernelName Name of the compute kernel function.
        /// @param rootDir Optional root directory.
        /// @return Compute pipeline state.
        MetalStuff::Pipeline::ComputePipelinePtr buildCompute(
            const char* filepath,
            const char* kernelName,
            const char* rootDir = NULL);
        //--------------------------------------------------------------------------------------------------------------

        MetalStuff::Pipeline::RenderPipelinePtr buildRender(
            MTL::PixelFormat pixelformat,
            MTL::PixelFormat depthPixelformat,
            uint32_t sampleCount,
            const char* filepath,
            const char* vertexName,
            const char* fragmentName, 
            const char* rootDir = NULL);
        //--------------------------------------------------------------------------------------------------------------

    private:

        /// @brief Metal device (non-owning reference)
        MTL::Device* mDevice = nullptr;
};
//----------------------------------------------------------------------------------------------------------------------