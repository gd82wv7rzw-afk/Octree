#include "Simulation/Shader/MetalNodes.h"
#include "Rendering/Vertices.h"

#include <metal_stdlib>
using namespace metal;
using namespace Simulation;
using namespace Rendering;

kernel void copyOutput(
    device   const LeafNode*     atoms   [[buffer(0)]],
    device         SimulationVertex* output  [[buffer(1)]],
    constant const float3&       box     [[buffer(2)]],
    uint id                [[thread_position_in_grid]]
) 
{
    auto atom = atoms[id].value;
    SimulationVertex& out = output[id];

    // bring the possitions in the range -1 .. +1
    out.pos.x = atom.pos.x * 1.5 / box.x;
    out.pos.y = atom.pos.y * 1.5 / box.y;
    out.pos.z = atom.pos.z * 1.5 / box.z;

    out.vel = atom.vel;
    out.acc = atom.acc;
}
//----------------------------------------------------------------------------------------------------------------------
