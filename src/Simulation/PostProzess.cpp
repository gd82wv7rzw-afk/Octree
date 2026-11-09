
#include "PostProzess.h"

#include <simd/simd.h>
#include <omp.h>

using namespace Simulation;
using namespace simd;
using namespace Rendering;

float PostProzess::eKin(Buffer* data)
{
    float ekin = 0.f;

    #pragma omp parallel for reduction(+:ekin)
    for (int i = 0; i < data->size(); ++i)
    {
        auto v = (*data)[i].vel;

        ekin += dot(v, v);
    }
    ekin /= data->size();
    printf("Ekin =%f\n", ekin);

    return ekin;
}
//----------------------------------------------------------------------------------------------------------------------