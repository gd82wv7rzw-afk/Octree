
#include <metal_stdlib>
using namespace metal;

struct VertexIn 
{
    float3 pos [[attribute(0)]];
    float3 vel [[attribute(1)]];
    float3 acc [[attribute(2)]];
};

struct VertexOut 
{
    float4 position [[position]];
};

//constant float PI = 3.14159265359;

vertex VertexOut vertexMain(
    uint vertexID [[vertex_id]],
    uint instanceID [[instance_id]],
    VertexIn in [[stage_in]])
{
    VertexOut out;

    constexpr float2 offsets[3] = {
        float2(-0.001, -0.001),
        float2( 0.001, -0.001),
        float2( 0.001,  0.001)
    };

    float2 pos = in.pos.xy + offsets[vertexID];

    out.position = float4(pos, in.pos.z, 1.0);

    return out;
}
//----------------------------------------------------------------------------------------------------------------------

fragment float4 fragmentMain(VertexOut in [[stage_in]])
{
    return float4(1,1,0,1);
}
//----------------------------------------------------------------------------------------------------------------------
