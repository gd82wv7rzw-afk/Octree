
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
    float epot = 0;

    #pragma omp parallel for reduction(+:ekin) reduction(min:min) reduction(max:max) reduction(+:epot)
    for (int i = 0; i < data->size(); ++i)
    {
        auto v  = (*data)[i].vel;
        auto a2 = dot(v, v);

        ekin += a2;
        if (a2 < min) min = a2;
        if (a2 > max) max = a2;    
        
        epot += (* data)[i].ePot;
    }
    ekin /= data->size();
    epot /= data->size();

    //printf("Ekin: %17.5f min: %17.5f max: %17.5f Epot:%17.5f\n", ekin, min, max, epot);

    return {ekin, min, max};
}
//----------------------------------------------------------------------------------------------------------------------