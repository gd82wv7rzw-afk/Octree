#include "Simulation/Shader/MetalNodes.h"

#include <metal_stdlib>
using namespace metal;


float3 calculatePeriodicDistance(thread const float3& a, thread const float3&b, constant const float3& box)
{
    float3 x = b - a;

    // Branchless: Wähle box.x, wenn Bedingung wahr ist, sonst 0
    x.x += select(0.0f, box.x, x.x < -0.5f * box.x);
    x.x -= select(0.0f, box.x, x.x >= 0.5f * box.x);

    x.y += select(0.0f, box.y, x.y < -0.5f * box.y);
    x.y -= select(0.0f, box.y, x.y >= 0.5f * box.y);

    x.z += select(0.0f, box.z, x.z < -0.5f * box.z);
    x.z -= select(0.0f, box.z, x.z >= 0.5f * box.z);

    return x;
}
//----------------------------------------------------------------------------------------------------------------------

//float maxDim(const thread float3& x)
//{
//    float max = 0;
//
//    if (fabs(x.x) > max) max = fabs(x.x);
//    if (fabs(x.y) > max) max = fabs(x.y);
//    if (fabs(x.z) > max) max = fabs(x.z);
//
//    return max;
//}
//----------------------------------------------------------------------------------------------------------------------

float3 grad(const thread float3& x)
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

float3 calculateImageWaitsL(thread float3& minP, thread float3& maxP, constant const float3& box, float width)
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
    thread const float3& r, 
    thread const float3& imageWaitsL, 
    thread const float3& imageWaitsR,
    constant const float3& box,
    const float wait,
    device float3& acc
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

kernel void calculateAccelleration(
    device         Simulation::LeafNode*   atoms          [[buffer(0)]],
    device   const Simulation::BranchNode* branches       [[buffer(1)]],
    constant const float3&                 box            [[buffer(2)]],
    constant const float&                  mTheta         [[buffer(3)]],
    threadgroup    Simulation::BranchNode* sharedBranches [[threadgroup(0)]], // NEU
    uint tid [[thread_position_in_grid]],
    uint lid [[thread_position_in_threadgroup]],         // NEU
    uint threadsPerGroup [[threads_per_threadgroup]])    // NEU

//    device         Simulation::LeafNode*   atoms    [[buffer(0)]],
//    device   const Simulation::BranchNode* branches [[buffer(1)]],
//    constant const float3&                 box      [[buffer(2)]],
//    constant const float&                  mTheta   [[buffer(3)]],
//    uint tid [[thread_position_in_grid]])
{
    using namespace Simulation;

    // 1. Shared Memory laden (wie zuvor)
    const uint maxSharedNodes = 128;
    if (lid < maxSharedNodes) 
    {
        sharedBranches[lid] = branches[lid];
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);

    // 2. Hilfsfunktion, um immer den richtigen Speicher zu erwischen
    auto getBranch = [&](uint idx) -> const BranchNode* 
    {
        if (idx < maxSharedNodes) 
        {
            return sharedBranches + idx;
        } else 
        {
            return branches + idx;
        }
    };

    // 3. Dein alter Code, aber wir nutzen jetzt getBranch(idx)
    atoms[tid].value.acc = {0.f, 0.f, 0.f};

    uint rootId  = atoms[tid].rootId;
    uint rootIdx = rootId & IndexMask;
    uint rootOct = rootId >> OctantOffsetBitNumber;

    // Hier nutzen wir das erste Mal unsere Hilfsfunktion!
    const BranchNode* root = getBranch(rootIdx);
    const BranchNode* wbranch = root;

    uint oct = 0;
    const float mTheta2 = mTheta * mTheta;

    while (wbranch != nullptr)
    {
        for (; oct < 8; ++oct)
        {
            // scip the own leaf
            if (wbranch == root && oct == rootOct) continue;

            // scip, if the leaf is empty
            if (wbranch->childIds[oct] == IndexMask) continue;

            bool isBranch = wbranch->childIds[oct] & BranchBit;
            uint childIdx = wbranch->childIds[oct] & IndexMask;

            // NEU: Wir holen uns den Kind-Knoten über unsere Hilfsfunktion
            const BranchNode* childBranch = isBranch ? getBranch(childIdx) : nullptr;

            // NEU: Zugriff über childBranch statt direkt über branches[...]
            float3 rOther = isBranch ? childBranch->value.com : atoms[childIdx].value.pos;
            float3 selfR  = atoms[tid].value.pos;
            float3 r      = calculatePeriodicDistance(selfR, rOther, box);

            // NEU: Auch hier nutzen wir childBranch
            float width   = isBranch ? childBranch->value.width : 0.f;

            float      invR2 = 1.f / dot(r,r);
            float      theta2 = (width * width) * invR2;
            const uint wait  = isBranch ? wbranch->childSizes[oct] : 1;

            if (theta2 > mTheta2)
            {
                // NEU: Auch beim Abstieg in den Baum getBranch nutzen
                wbranch = getBranch(childIdx);
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

                calculateClusterAcceleration(r, imageWaitsL, imageWaitsR, box, wait, atoms[tid].value.acc);
            }
            else
            {
                atoms[tid].value.acc += grad(r);
            }
        }

        // we are in a child node and jumb to the had of the outer loop.
        if (oct < 8) continue;

        // we are at the end and we go one order higher
        if (wbranch == root)
        {
            rootId  = root->rootId;
            rootIdx = rootId & IndexMask;
            rootOct = rootId >> OctantOffsetBitNumber;

            root    = rootIdx != IndexMask ? branches + rootIdx : nullptr;
            wbranch = root;

            // we start at 0
            oct     = 0;
        }

        // only the currend working branch is at the end and have to go up.
        else
        {
            uint id  = wbranch->rootId;
            uint idx = id & IndexMask;

            // we start at the next octant
            oct      = (id >> OctantOffsetBitNumber) + 1;
            wbranch  = idx != IndexMask ? branches + idx : nullptr;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

