
#include "PostProzess.h"

#include <simd/simd.h>
#include <omp.h>

using namespace Simulation;
using namespace simd;
using namespace Rendering;

float3 PostProzess::eKin(Buffer* data)
{
    float min = 1e3;
    float max = -1e3;
    float ekin = 0;

    #pragma omp parallel for reduction(+:ekin) reduction(min:min) reduction(max:max)
    for (int i = 0; i < data->size(); ++i)
    {
        auto v  = (*data)[i].vel;
        auto a2 = dot(v, v);

        ekin += a2;
        if (a2 < min) min = a2;
        if (a2 > max) max = a2;        
    }
    ekin /= data->size();
    //printf("Ekin =%f\n", ekin);

    return {ekin, min, max};
}
//----------------------------------------------------------------------------------------------------------------------