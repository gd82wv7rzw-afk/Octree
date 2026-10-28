#include "Simulation/Shader/MetalNodes.h"

#include <metal_stdlib>
using namespace metal;
using namespace Simulation;

kernel void findReinstertAtoms(
    device const LeafNode*   atoms        [[buffer(0)]],
    device const BranchNode* branches     [[buffer(1)]],
    device       int*        neadReinsert [[buffer(2)]],
    uint id                 [[thread_position_in_grid]]
) 
{
    const LeafNode   atom = atoms[id];
    const int      rootId = atom.rootId & IndexMask;
    const BranchNode root = branches[rootId];
    const float width     = root.value.width;
    const float3 minPos   = floor( root.value.com / width) * width;
    const float3 maxPos   = minPos + width;
    const float3        r = atom.value.pos;

    neadReinsert[id] = -1;
    for( int i = 0; i < 3; ++i) 
    {
        if( r[i] < minPos[i] || r[i] >= maxPos[i])
        {
            neadReinsert[id] = id;
            return;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------