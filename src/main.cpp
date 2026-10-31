
#include "Simulation/MetalGravity.h"

#include <chrono>
#include <simd/simd.h>

int main(void) {

    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();

    const unsigned nAtoms  = 1e5;
    const float freeLength = 1e3;

    Simulation::MetalGravity gravitySim(device, queue, nAtoms, freeLength);
    gravitySim.generateRandomConfiguration();

    for ( int i = 0; i < 100000; ++i)
    {
        if (i % 100 == 0) gravitySim.printSnapshot(i);

        auto start = std::chrono::steady_clock::now();
        gravitySim.step();
        auto end = std::chrono::steady_clock::now();
        auto time = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6;

        printf("Step: %7i\tTime: %14.7lfs\n", i, time);
    }

    queue->release();
    device->release();

    return 0;
}