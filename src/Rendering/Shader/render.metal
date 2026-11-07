
#include <metal_stdlib>
#include "Rendering/CameraUniforms.h"
#include "Rendering/Vertices.h"

using namespace metal;
using namespace Rendering;

struct VertexOut
{
    float4 position [[position]];
};
//----------------------------------------------------------------------------------------------------------------------

vertex VertexOut vertexMain(
    PNVertex in [[stage_in]],
    constant CameraUniforms& camera [[buffer(1)]])
{
    VertexOut out;

    float4 worldPosition = float4(in.pos, 1.0);
    //out.position = worldPosition;
    out.position = camera.projection * camera.view * worldPosition;

    return out;
}
//----------------------------------------------------------------------------------------------------------------------

fragment float4 fragmentMain(VertexOut in [[stage_in]])
{
    return float4(1,0,0,1);
}
//----------------------------------------------------------------------------------------------------------------------
