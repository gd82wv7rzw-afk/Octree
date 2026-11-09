
#include "CpuForce.h"

#include <omp.h>

using namespace Simulation;
using namespace simd;

namespace {
    
float3 calculatePeriodicDistance(const float3& a, const float3& b, const float3& box)
{
    float3 x = b - a;

    // Branchless: Wähle box.x, wenn Bedingung wahr ist, sonst 0
    x.x += (x.x < -0.5f * box.x) ? box.x : 0; //simd::select(0.0f, box.x, x.x < -0.5f * box.x);
    x.x -= (x.x >= 0.5f * box.x) ? box.x : 0; //simd::select(0.0f, box.x, x.x >= 0.5f * box.x);

    x.y += (x.y < -0.5f * box.y) ? box.y : 0; //simd::select(0.0f, box.y, x.y < -0.5f * box.y);
    x.y -= (x.y >= 0.5f * box.y) ? box.y : 0; //simd::select(0.0f, box.y, x.y >= 0.5f * box.y);

    x.z += (x.z < -0.5f * box.z) ? box.z : 0; //simd::select(0.0f, box.z, x.z < -0.5f * box.z);
    x.z -= (x.z >= 0.5f * box.z) ? box.z : 0; //simd::select(0.0f, box.z, x.z >= 0.5f * box.z);

    return x;
}
//----------------------------------------------------------------------------------------------------------------------

float3 grad(const float3& x)
{
    auto r2 = dot(x,x) + 1e-10f;
    
    // rsqrt(r2) ist 1/r. Das ist extrem schnell auf Apple Silicon.
    auto invR  = rsqrt(r2);             // 1/r
    auto invR3 = invR * invR * invR;    // 1/r^3
    auto invR4 = invR3 * invR;          // 1/r^4

    auto factor = -2.f * invR4 + invR3;

    return x * factor;
}
//----------------------------------------------------------------------------------------------------------------------

float3 calculateImageWaitsL(float3& minP, float3& maxP, const float3& box, float width)
{
    float3 imageWaitsL = {0.f, 0.f, 0.f};

    for(int i = 0; i < 3; ++i) 
    {
        if (minP[i] < -0.5f * box[i]) 
        {
            minP[i] += box[i];

            imageWaitsL[i] = (box[i] * 0.5f - minP[i]) / width;
            if (imageWaitsL[i] > 1) imageWaitsL[i] = 1;
            
            if (maxP[i] < -0.5f * box[i]) maxP[i] += box[i];
        }
        if (minP[i] >= 0.5 * box[i]) 
        {
            minP[i] -= box[i];

            imageWaitsL[i] = (minP[i] - 0.5f * box[i]) / width;
            if (imageWaitsL[i] > 1) imageWaitsL[i] = 1;
            
            if (maxP[i] >= 0.5 * box[i]) maxP[i] -= box[i];
        }       
    }

    return imageWaitsL;
}
//----------------------------------------------------------------------------------------------------------------------

void calculateClusterAcceleration(
    const float3& r, 
    const float3& imageWaitsL, 
    const float3& imageWaitsR,
    const float3& box,
    const float wait,
    float3& acc
)
{
    for(int mask = 0; mask < 8; ++mask)
    {
        float3 rImage      = r;
        float  imageWeight = 1.0f;
        bool   validImage  = true;

        for (uint i = 0; i < 3; ++i)
        {
            if (mask & (1u << i))
            {
                if (imageWaitsR[i] < FLT_EPSILON)
                {
                    validImage = false;
                    break;
                }

                imageWeight *= imageWaitsR[i];
                rImage[i] += (r[i] < 0.0f) ? box[i] : -box[i];
            }
            else
            {
                if (imageWaitsL[i] < FLT_EPSILON)
                {
                    validImage = false;
                    break;
                }

                imageWeight *= imageWaitsL[i];
            }
        }

        if (!validImage) continue;

        acc += grad(rImage) * (wait * imageWeight);
    }
}
//----------------------------------------------------------------------------------------------------------------------
}

void CpuForce::calculateAcceleration(Atom* atom, Tree& tree, const float3& box, float theta, int atomid)
{
    auto parentIndex = tree.leafs()->getPointer(atomid)->parentIndex();
    atom->acc = float3{0.f,0.f,0.f};
    auto root = *(Tree::const_iterator(&tree, parentIndex));
    auto wbranch = root;
    uint rootOct = tree.leafs()->getPointer(atomid)->possitonInParent();
    auto mTheta2 = theta * theta;

    uint oct = 0;
    while (wbranch != nullptr)
        {
            for (; oct < 8; ++oct)
            {
                // scip the own leaf
                if (wbranch == root && oct == rootOct) continue;
 
                // scip, if the leaf is empty
                if (wbranch.isEmpty(oct)) continue;
 
                bool isBranch = wbranch.isBranch(oct);
                uint childIdx = wbranch.index(oct);
 
                float3 rOther = isBranch ? wbranch.branch(oct).value().com : wbranch.leaf(oct).value().pos;
                float3 selfR  = atom->pos;
                float3 r      = calculatePeriodicDistance(selfR, rOther, box);
 
                // NEU: Auch hier nutzen wir childBranch
                float width   = isBranch ? wbranch.branch(oct).value().width : 0.f;
 
                float      invR2 = 1.f / dot(r,r);
                float      theta2 = (width * width) * invR2;
                const uint wait  = isBranch ? wbranch.size(oct) : 1;
 
                if (theta2 > mTheta2)
                {
                    wbranch.goIn(oct);
                    oct = 0;
                    break;
                }
 
                if (isBranch)
                {
                    float3 minPos = floor( rOther / width) * width;
                    float3 maxPos = minPos + width;
 
                    float3 minR = minPos - selfR;
                    float3 maxR = maxPos - selfR;
 
                    float3 imageWaitsL = calculateImageWaitsL(minR, maxR, box, width);
                    float3 imageWaitsR = float3{1.f, 1.f, 1.f} - imageWaitsL;
 
                    calculateClusterAcceleration(r, imageWaitsL, imageWaitsR, box, wait, atom->acc);
                }
                else
                {
                    atom->acc += grad(r);
                }
            }
 
            // we are in a child node and jumb to the had of the outer loop.
            if (oct < 8) continue;
 
            // we are at the end and we go one order higher
            if (wbranch == root)
            {
                oct = 0;
                rootOct = wbranch.self().possitonInParent();
                root.goUp();
                wbranch = root;
            }
 
            // only the currend working branch is at the end and have to go up.
            else
            {
                oct = wbranch.self().possitonInParent() + 1;
                wbranch.goUp();
            }
        }
}

void CpuForce::calculateAcceleration(atomArry& atoms, Tree& tree, int nAtoms, const float3& box, float theta)
{
    auto begin = tree.root().begin();

    const int nThreads = omp_get_max_threads();

    #pragma omp parallel num_threads(nThreads)
    {
        const int tid = omp_get_thread_num();

        const int first = nAtoms * tid       / nThreads;
        const int last  = nAtoms * (tid + 1) / nThreads;

        auto it = begin;
        std::advance(it, first);

        for (int i = first; i < last; ++i, ++it)
        {
            calculateAcceleration(&atoms[i].value(), tree, box, theta, i);
        }
    }
}