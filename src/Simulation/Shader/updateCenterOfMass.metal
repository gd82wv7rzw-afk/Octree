#include "Simulation/Shader/MetalNodes.h"

#include <metal_stdlib>
using namespace metal;
using namespace Simulation;

kernel void updateCenterOfMass(
    device   const LeafNode*   atoms     [[buffer(0)]],
    device         BranchNode* branches  [[buffer(1)]],
    constant const int&       order      [[buffer(2)]],
    uint id                [[thread_position_in_grid]]
) 
{
    BranchNode& branch = branches[id];
    if (branch.value.order != order) return;
    
    float3 centerOfMass = float3(0.0f);
    float  maxVel       = 0.f;
    float  maxAcc       = 0.f;

    int n = 0;
    for (int o = 0; o < 8; ++o)
    {
        if (branch.childSizes[o] == 0) continue;

        bool isBranch = branch.childIds[o] & BranchBit;
        auto idx      = branch.childIds[o] & IndexMask;

        float3 r  = isBranch ? branches[idx].value.com : atoms[idx].value.pos;
        float vel = isBranch ? branches[idx].value.maxVel : length(atoms[idx].value.vel);
        float acc = isBranch ? branches[idx].value.maxAcc : length(atoms[idx].value.acc);
        auto  sz  = branch.childSizes[o];
        
        maxAcc = max(maxAcc, acc);
        maxVel = max(maxVel, vel);
        
        centerOfMass += r * sz;

        n += sz;
    }

    branch.value.com    = centerOfMass / n;
    branch.value.maxVel = maxVel;
    branch.value.maxAcc = maxAcc;
}
//----------------------------------------------------------------------------------------------------------------------
