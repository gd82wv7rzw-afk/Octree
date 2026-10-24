
#include "SimulatorV0.h"
#include "Simulation/MetalGravity.h"

#include <chrono>
#include <iostream>
#include <ratio>
#include <simd/quaternion.h>
#include <simd/simd.h>
#include <simd/vector_types.h>

int main(void) {

//  MTL::Device* device = MTL::CreateSystemDefaultDevice();
//  MTL::CommandQueue* queue = device->newCommandQueue();

//  Simulation::MetalGravity gravitySim(device, queue);
//  gravitySim.generateRandomConfiguration({1.f, 1.f, 1.f}, 1e5);

//  printf("%d\n\n", (int)1e5);
//  for (auto a: gravitySim.tree()->root())
//  {
//      auto r = a.value().pos;
//      printf("Xx %9.5f\t%9.5f\t%9.5f\n", r.x, r.y, r.z);
//  }

//  queue->release();
//  device->release();

    SimulatorV0 simul;
    using namespace simd;

    auto start = std::chrono::steady_clock::now();
    simul.init();
    auto end   = std::chrono::steady_clock::now();
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