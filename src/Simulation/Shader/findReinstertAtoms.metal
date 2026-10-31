#include "Simulation/Shader/MetalNodes.h"

#include <metal_stdlib>
using namespace metal;
using namespace Simulation;

kernel void findReinstertAtoms(
    device       LeafNode*   atoms        [[buffer(0)]],
    device const BranchNode* branches     [[buffer(1)]],
    device       int*        rootSize     [[buffer(2)]],
    device       LeafNode*   atomsBackup  [[buffer(3)]],
    constant const float3&   box          [[buffer(4)]],
    uint id                 [[thread_position_in_grid]]
) 
{
    LeafNode& atom = atoms[id];
    
    const int      rootId = atom.rootId & IndexMask;
    const BranchNode root = branches[rootId];
    const float width     = root.value.width;
    const float3 minPos   = root.value.min;
    const float3 maxPos   = minPos + float3{width, width, width};
    float3              r = atom.value.pos;
    const float3    box_2 = box / 2;

    // periodic boundery conditions
    for( int i = 0; i < 3; ++i) 
    {
        float pos = r[i];

        while( pos < -box_2[i])  pos += box[i]; 
        while( r[i] >= box_2[i]) pos -= box[i];  
        
        atom.value.pos[i] = pos;
    }

    atomsBackup[id]  = atom;
    r                = atom.value.pos;

    rootSize[id] = 0;
    for( int i = 0; i < 3; ++i) 
    {
        if( r[i] < minPos[i] || r[i] >= maxPos[i])
        {
            rootSize[id] = 1;         
            break;
        }
    }   
}
//----------------------------------------------------------------------------------------------------------------------