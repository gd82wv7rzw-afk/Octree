
#include "SimulatorV0.h"
#include "Simulation/MetalGravity.h"

#include <chrono>
#include <iostream>
#include <ratio>
#include <simd/quaternion.h>
#include <simd/simd.h>
#include <simd/vector_types.h>

int main(void) {

    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();
    const unsigned nAtoms  = 1e5;
    const float freeLength = 1e3;

    auto start = std::chrono::steady_clock::now();
    Simulation::MetalGravity gravitySim(device, queue, nAtoms, freeLength);
    gravitySim.generateRandomConfiguration();
    auto end   = std::chrono::steady_clock::now();
    auto InitDuration = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6;

    printf("Init time:   %9.5lfs\n", InitDuration);

    start = std::chrono::steady_clock::now();
    gravitySim.step();
    end   = std::chrono::steady_clock::now();
    auto GPUDuration = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6;

    printf("step time: %9.5lfs\n", GPUDuration);


//  printf("%d\n%9.5lfs\n", (int)nAtoms, GPUDuration);
//  for (auto a: gravitySim.tree()->root())
//  {
//      auto r = a.value().pos;
//      auto acc = a.value().acc;
//      printf("Xx %9.5f\t%9.5f\t%9.5f\t%9.5f\t%9.5f\t%9.5f\n", r.x, r.y, r.z, acc.x, acc.y, acc.z);
//  }
//  printf("%d\n%9.5lfs\n", (int)nAtoms, GPUDuration);

    queue->release();
    device->release();
    return 0;

    SimulatorV0 simul;
    using namespace simd;

    start = std::chrono::steady_clock::now();
    simul.init();
    end   = std::chrono::steady_clock::now();
    auto initDuration = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6;
    //printf("init   Duration: %9.5lfs\n", initDuration);

    start = std::chrono::steady_clock::now();
    simul.calculateAcceleration(simul.mTree);
    end   = std::chrono::steady_clock::now();
    auto treeDuration = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6;
    //printf("aTree: duration: %9.5lfs\n", treeDuration);

    simul.printSnapShot();
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; ++i) 
    {
        //simul.printSnapShot();
        simul.aniling();
        simul.updateTree(simul.mTree);
        simul.calculateAcceleration(simul.mTree);
    }

    simul.mDt *= 0.5;
 
    for (int i = 100; i < 100000; ++i) 
    {
        start = std::chrono::steady_clock::now();
        if ( (i % 500) == 0) 
        {
            simul.printSnapShot();
        }
        simul.step();
        end   = std::chrono::steady_clock::now();
        std::cerr << "step" << i << "step duration:" << std::chrono::duration<double, std::micro>(end - start).count() * 1e-6 << std::endl;
    }

    return 0;
}