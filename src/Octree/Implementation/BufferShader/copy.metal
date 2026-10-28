#include <metal_stdlib>
using namespace metal;

kernel void copy(
    device const uchar* source [[buffer(0)]],
    device uchar* destination  [[buffer(1)]],
    constant uint& stride      [[buffer(2)]],
    uint id                    [[thread_position_in_grid]]
)
{
    uint offset = id * stride;

    for (uint i = 0; i < stride; ++i) {
        destination[offset + i] = source[offset + i];
    }
}
//----------------------------------------------------------------------------------------------------------------------