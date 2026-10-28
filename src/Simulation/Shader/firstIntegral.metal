#include "Simulation/Shader/MetalNodes.h"

#include <metal_stdlib>
using namespace metal;
using namespace Simulation;


kernel void firstIntegral(
    device   const LeafNode* atoms     [[buffer(0)]],
    device         LeafNode* nextAtoms [[buffer(1)]],
    constant       float&    dt        [[buffer(2)]],
    uint id              [[thread_position_in_grid]]
) 
{
    const float3 r = atoms[id].value.pos;
    const float3 v = atoms[id].value.vel; 
    const float3 a = atoms[id].value.acc;  
    
    nextAtoms[id].value.pos = r + v * dt + a * dt * dt * 0.5f;
}
//----------------------------------------------------------------------------------------------------------------------

kernel void secondIntegralAndErrors(
    device   const LeafNode* atomsA [[buffer(0)]],
    device         LeafNode* atomsB [[buffer(1)]],
    device         float* errors    [[buffer(2)]],
    constant       float& dt        [[buffer(3)]],
    uint id    [[thread_position_in_grid]]
) 
{
    const float3 rA = atomsA[id].value.pos;
    const float3 rB = atomsB[id].value.pos;
    const float3 v  = atomsA[id].value.vel; 
    const float3 aA = atomsA[id].value.acc;  
    const float3 aB = atomsB[id].value.acc;

    float3 rNew  = rA + v * dt + (aA / 3 + aB / 6) * dt * dt;
    float  delta = length(rNew - rB);

    atomsB[id].value.pos = rNew;
    errors[id]           = delta / dt;
}
//----------------------------------------------------------------------------------------------------------------------

kernel void integrateVeloseties(
    device   const LeafNode* atomsA [[buffer(0)]],
    device         LeafNode* atomsB [[buffer(1)]],
    constant       float& dt        [[buffer(2)]],
    uint id    [[thread_position_in_grid]]
) 
{
    const float3 v  = atomsA[id].value.vel; 
    const float3 aA = atomsA[id].value.acc;  
    const float3 aB = atomsB[id].value.acc;

    atomsB[id].value.vel = v + (aA + aB) * 0.5f * dt;
}
//----------------------------------------------------------------------------------------------------------------------