
#include <metal_stdlib>

#include "Rendering/Vertices.h"

using namespace metal;
using namespace Rendering;

struct VertexOut
{
    float4 position [[position]];
};
//----------------------------------------------------------------------------------------------------------------------

vertex VertexOut vertexMain
(
    PNVertex in                      [[stage_in]],
    constant SimulationVertex* atoms [[buffer(1)]],
    constant float4x4& pvm           [[buffer(2)]],
    uint instanceID                  [[instance_id]]
)
{
    VertexOut out;
    const float scale = 1e-3;

    float3 atomPos  = atoms[instanceID].pos;
    float3 worldPos = in.pos * scale + atomPos;

    out.position = pvm * float4(worldPos, 1.0f);

    return out;
}
//----------------------------------------------------------------------------------------------------------------------

fragment float4 fragmentMain(VertexOut in [[stage_in]])
{
    return float4(1,0,0,1);
}
//----------------------------------------------------------------------------------------------------------------------
