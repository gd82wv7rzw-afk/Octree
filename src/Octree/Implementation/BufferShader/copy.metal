#include <metal_stdlib>
using namespace metal;

kernel void copy(
    device const uchar* source [[buffer(0)]],
    device uchar* destination  [[buffer(1)]],
    constant uint& size        [[buffer(2)]],
    uint id                    [[thread_position_in_grid]]
)
{
    if (id < size) {
        destination[id] = source[id];
    }
}