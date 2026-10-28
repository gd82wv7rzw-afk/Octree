#include <metal_stdlib>

using namespace metal;

kernel void reduceMaxError(
    device float* values   [[buffer(0)]],
    constant uint& count   [[buffer(1)]],
    uint gid               [[thread_position_in_grid]])
{
    uint left = gid * 2;

    if (left >= count) return;

    float m = values[left];

    if (left + 1 < count) m = max(m, values[left + 1]);

    values[gid] = m;
}
//----------------------------------------------------------------------------------------------------------------------