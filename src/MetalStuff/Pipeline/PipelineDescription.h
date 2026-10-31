
#pragma once

#include <Metal/Metal.hpp>

namespace MetalStuff::Pipeline {

/// @brief Custom deleter for MTL::RenderPipelineState.
/// @details
/// Used by std::unique_ptr to properly release Metal pipeline state objects
/// via reference counting. Ensures RAII-compliant GPU resource management.
struct ComputePipelineDeleter 
{
    /// @brief Releases a RenderPipelineState if valid.
    /// @param p Pointer to Metal RenderPipelineState.
    // Metal C++ Ownership Release
    inline void operator()(MTL::ComputePipelineState* p) const 
    { 
        if (p) p->release(); 
        p = nullptr;
    } 
};
//----------------------------------------------------------------------------------------------------------------------

/// @brief Smart pointer type for owning RenderPipelineState objects.
/// @details
/// Encapsulates Metal pipeline lifetime management using RAII.
/// Automatically releases GPU resources via PipelineDeleter.
using ComputePipelinePtr = std::unique_ptr<MTL::ComputePipelineState, ComputePipelineDeleter>;
//----------------------------------------------------------------------------------------------------------------------

} // End namespace Pipeline