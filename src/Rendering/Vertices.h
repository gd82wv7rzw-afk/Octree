#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
#else
    #include <simd/simd.h>
#endif

namespace Rendering {

/// @brief Defines how vertex memory is interpreted by the pipeline.
/// @details
/// This is the single source of truth for vertex layouts in the engine.
/// Metal mapping is handled later in the renderer backend.
enum class VertexFormat : uint8_t
{
    /// @brief position normals
    PNVertex    = 0,

    /// @brief position normals uv-coordinats
    PNTVertex   = 1,

    SimulationVertex = 2,  

    PNTCVertex = 3
};
//----------------------------------------------------------------------------------------------------------------------

struct SimulationVertex 
{
#ifdef __METAL_VERSION__
    float3 pos [[attribute(0)]];
    float3 vel [[attribute(1)]];
    float3 acc [[attribute(2)]];
    float ePot [[attribute(3)]];
#else
    simd::float3 pos{0.f, 0.f, 0.f};
    simd::float3 vel{0.f, 0.f, 0.f};
    simd::float3 acc{0.f, 0.f, 0.f};
    float       ePot{0.f};
#endif
};
//----------------------------------------------------------------------------------------------------------------------

struct PNVertex
{
#ifdef __METAL_VERSION__
    float3 pos     [[attribute(0)]];
    float3 normale [[attribute(1)]];
#else
    simd::float3 pos{0.f, 0.f, 0.f};
    simd::float3 normale{0.f, 0.f, 0.f};
#endif
};
// ---------------------------------------------------------------------------------------------------------------------

struct PNTVertex
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texture;
};
// ---------------------------------------------------------------------------------------------------------------------

struct PNTCVertex
{
    simd::float3 position;
    simd::float3 normal;
    simd::float2 texture;
    simd::float3 color;
};
// ---------------------------------------------------------------------------------------------------------------------
} // namespace Renderering ---------------------------------------------------------------------------------------------