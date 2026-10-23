
#include "SimulatorV0.h"

#include <chrono>
#include <iostream>
#include <ratio>
#include <simd/quaternion.h>
#include <simd/simd.h>
#include <simd/vector_types.h>


int main(void) {

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

    //simul.printSnapShot();
    //start = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; ++i) 
    {
        //simul.printSnapShot();
        simul.aniling();
        simul.updateTree(simul.mTree);
        simul.calculateAcceleration(simul.mTree);
    }

    simul.mDt *= 0.5;

    for (int i = 100; i < 1000000; ++i) 
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