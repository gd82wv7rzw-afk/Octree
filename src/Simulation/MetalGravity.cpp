#include "MetalGravity.h"

#include<list>

using namespace Simulation;
using namespace simd;
using namespace std;

MetalGravity::MetalGravity(MTL::Device* device, MTL::CommandQueue* que, int nAtoms, float freelength) noexcept
    : mDevice{device}
    , mQueue{que}
    , mRandom{std::random_device{}()} 
    , mTree(Tree::branchBuffer_type(device, que), Tree::leafBuffer_type(device,que))
    , mNextAtoms(device, que)
    , mMaxError(device, nAtoms)
    , mNeadReinsert(device, nAtoms)
{
    mNatoms            = nAtoms;
    const float boxDim = cbrt(1.f*nAtoms) * freelength;
    mBox = float3{boxDim, boxDim, boxDim};

    buildPipelines();
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::generateRandomConfiguration()
{
    // allocate memory
    mTree.branches()->allocate(mNatoms / 2);
    mTree.leafs()->allocate(mNatoms);

    // generate random points and veloseties inside the box
    for (int i = 0; i < mNatoms; ++i) 
    {
        float3 point;

        while (true)
        {
            float x = std::normal_distribution<float>(0.f, mBox.x / 10)(mRandom);
            float y = std::normal_distribution<float>(0.f, mBox.y / 10)(mRandom);
            float z = std::normal_distribution<float>(0.f, mBox.z / 10)(mRandom);

            float3 p{x, y, z};

            if (dot(p,p) < mBox.x * mBox.x / 4) 
            {
                point = p;
                break;
            }
            printf("random point: %f\t%f\n", length(p), length(mBox/2));
        };  
              
        float3 v1
        {
            std::normal_distribution<float>(0.0f, 1.f)(mRandom),
            std::normal_distribution<float>(0.0f, 1.f)(mRandom),
            std::normal_distribution<float>(0.0f, 1.f)(mRandom)
        }; 

        v1 /= mNatoms;

        Atom atom{point, v1, {0.f,0.f,0.f}};

        insertAtomInTree(mTree, atom);
    }

    mNextAtoms = *(mTree.leafs());

    //accelaeration
    auto* commandBuffer = mQueue->commandBuffer();
    auto* encoder       = commandBuffer->computeCommandEncoder();

    updateCenterOfMass(encoder);
    calculateAcelation(encoder, mTree.leafs()->metalBuffer());

    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    //encoder->release();
    //commandBuffer->release();
}
//----------------------------------------------------------------------------------------------------------------------

MetalGravity::Tree::reference::iterator MetalGravity::insertAtomInTree(Tree& tree, const Atom& atom)
{
    float3 center{0.0f, 0.0f, 0.0f};
    float3 offset{-0.5f*mBox.x, -0.5f*mBox.y, -0.5f*mBox.z}; 

    char   oct   = (atom.pos.x >= center.x) | (atom.pos.y >= center.y) << 1 | (atom.pos.z >= center.z) << 2;
    float  width = 0.5f*mBox.x; 
    int    order = 1;

    auto sub = tree.root();

    // only the first time
    if (tree.numberOfBranches() == 0) 
    {
        auto leaf = sub.newLeaf(oct);
        leaf->value() = atom;

        sub.self().value().com   = atom.pos;
        sub.self().value().min   = offset;
        sub.self().value().width = width * 2;
        sub.self().value().order = order;

        return leaf;
    }

    if (sub.isEmpty(oct)) 
    {
        auto leaf = sub.newLeaf(oct);
        leaf->value() = atom;
        
        return leaf;
    }
    else 
    {
        while (sub.isBranch(oct)) 
        {
            //sub.goIn(oct);
            auto tmp = sub.branch(oct).value();

            offset.x = oct & 0b001 ? offset.x + width: offset.x;
            offset.y = oct & 0b010 ? offset.y + width: offset.y;
            offset.z = oct & 0b100 ? offset.z + width: offset.z;

            width /= 2;

            center = offset + float3{width, width, width};

            sub.goIn(oct);
            oct = (atom.pos.x >= center.x) | (atom.pos.y >= center.y) << 1 | (atom.pos.z >= center.z) << 2;
        }

        if (sub.isLeaf(oct)) 
        {
            auto leafBack = sub.leaf(oct).value();
            sub.deleate(oct);
            sub.newSubTree(oct);

            sub.goIn(oct);
            sub.self().value().order = 1;

            offset.x = oct & 0b001 ? offset.x + width: offset.x;
            offset.y = oct & 0b010 ? offset.y + width: offset.y;
            offset.z = oct & 0b100 ? offset.z + width: offset.z;

            sub.self().value().width = width;
            sub.self().value().min   = offset;
            width /= 2;

            center = offset + float3{width, width, width};
           
            oct = (leafBack.pos.x >= atom.pos.x) | (leafBack.pos.y >= atom.pos.y) << 1 | (leafBack.pos.z >= atom.pos.z) <<2;
 
            sub.newLeaf(oct);
            sub.leaf(oct).value() = leafBack;

            oct = ~oct & 7;
 
            auto leaf = sub.newLeaf(oct);
            leaf->value() = atom;

            incrOrder(mTree, sub);

            return leaf;
        }           
        else
        {
            auto leaf = sub.newLeaf(oct);
            leaf->value() = atom;
            
            return leaf;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::incrOrder(Tree& tree, Tree::reference sub)
{
    int order = 1;
    while (sub != tree.end())
    {
        if (sub.self().value().order < order)
        {
            sub.self().value().order = order;
        }
        sub.goUp();
        ++order;
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::decrOrder(Tree& tree, Tree::reference sub)
{
    int order = 1;
    while (sub != tree.end())
    {
        int maxOrder = 0;

        for (int o = 0; o < 8; ++o) 
        { 
            if (sub.isBranch(o)) 
            {
                auto tmpO = sub.branch(o).value().order;
                if(tmpO > maxOrder) maxOrder = tmpO;
            }
        }

        order = maxOrder + 1;
        if (sub.self().value().order > order)
        {
            sub.self().value().order = order;
        }
        sub.goUp();
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::buildPipelines()
{
    assert(mDevice != nullptr);

    PipelineBuilder builder(mDevice);
    mCalculateAcelationPipeline = builder.buildCompute("/default.metallib", "calculateAccelleration");
    mFirstIntegralPipeline = builder.buildCompute("/default.metallib", "firstIntegral");
    mUpdateCenterOfMassPipeline = builder.buildCompute("/default.metallib", "updateCenterOfMass");
    mReduceMaxErrorPipeline = builder.buildCompute("/default.metallib", "reduceMaxError");
    mSecondIntegralPipeline = builder.buildCompute("/default.metallib", "secondIntegralAndErrors");
    mIntegratteVelosetiesPipeline = builder.buildCompute("/default.metallib", "integrateVeloseties");
    mFindReinsertAtomsPipeline = builder.buildCompute("/default.metallib", "findReinstertAtoms");
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::calculateAcelation(MTL::ComputeCommandEncoder* encoder, MTL::Buffer* atoms)
{
    encoder->setComputePipelineState(mCalculateAcelationPipeline.get());

    // kernel arguments
    encoder->setBuffer(atoms, 0, 0);
    encoder->setBuffer(mTree.branches()->metalBuffer(), 0, 1);
    encoder->setBytes(&mBox, sizeof(mBox), 2);
    encoder->setBytes(&mTheta, sizeof(mTheta), 3);
    //encoder->setBytes(&mR0square, sizeof(mR0square), 4);

    // NEU: Reserviere Speicher für 256 BranchNodes im Threadgroup-Speicher
    // Der Index 0 entspricht dem [[threadgroup(0)]] im Shader
    size_t sharedMemorySize = sizeof(TreeBranch) * 128;
    encoder->setThreadgroupMemoryLength(sharedMemorySize, 0);

    // gpu layout
    //const auto width = mCalculateAcelationPipeline->threadExecutionWidth();
    const auto width = 128;
    MTL::Size threadsPerThreadgroup(width, 1, 1);
    MTL::Size grid(mNatoms, 1, 1);

    encoder->dispatchThreads(grid, threadsPerThreadgroup);
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::firstIntegral(MTL::ComputeCommandEncoder* encoder)
{
    encoder->setComputePipelineState(mFirstIntegralPipeline.get());

    encoder->setBuffer(mTree.leafs()->metalBuffer(), 0, 0);
    encoder->setBuffer(mNextAtoms.metalBuffer(), 0, 1);
    encoder->setBytes(&mDt, sizeof(mDt), 2);

    // gpu layout
    const auto width = mFirstIntegralPipeline->threadExecutionWidth();
    MTL::Size threadsPerThreadgroup(width, 1, 1);
    MTL::Size grid(mNatoms, 1, 1);

    encoder->dispatchThreads(grid, threadsPerThreadgroup);
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::secondIntegralAndError(MTL::ComputeCommandEncoder* encoder)
{
    encoder->setComputePipelineState(mSecondIntegralPipeline.get());

    encoder->setBuffer(mTree.leafs()->metalBuffer(), 0, 0);
    encoder->setBuffer(mNextAtoms.metalBuffer(), 0, 1);
    encoder->setBuffer(mMaxError.buffer(), 0, 2);
    encoder->setBytes(&mDt, sizeof(mDt), 3);

    // gpu layout
    const auto width = mSecondIntegralPipeline->threadExecutionWidth();
    MTL::Size threadsPerThreadgroup(width, 1, 1);
    MTL::Size grid(mNatoms, 1, 1);

    encoder->dispatchThreads(grid, threadsPerThreadgroup);
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::integrateVelosseties(MTL::ComputeCommandEncoder* encoder)
{
    encoder->setComputePipelineState(mIntegratteVelosetiesPipeline.get());

    encoder->setBuffer(mTree.leafs()->metalBuffer(), 0, 0);
    encoder->setBuffer(mNextAtoms.metalBuffer(), 0, 1);
    encoder->setBytes(&mDt, sizeof(mDt), 2);

    // gpu layout
    const auto width = mIntegratteVelosetiesPipeline->threadExecutionWidth();
    MTL::Size threadsPerThreadgroup(width, 1, 1);
    MTL::Size grid(mNatoms, 1, 1);

    encoder->dispatchThreads(grid, threadsPerThreadgroup);
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::updateCenterOfMass(MTL::ComputeCommandEncoder* encoder)
{
    const auto maxOrder = mTree.root().self().value().order;
    int order = 1;

    encoder->setComputePipelineState(mUpdateCenterOfMassPipeline.get());

    encoder->setBuffer(mNextAtoms.metalBuffer(), 0, 0);
    encoder->setBuffer(mTree.branches()->metalBuffer(), 0, 1);
    encoder->setBytes(&order, sizeof(order), 0, 2);

    // gpu layout
    auto width = mUpdateCenterOfMassPipeline->threadExecutionWidth();
    MTL::Size threadsPerThreadgroup(width, 1, 1);
    MTL::Size grid(mTree.numberOfBranches(), 1, 1);

    encoder->dispatchThreads(grid, threadsPerThreadgroup);

    for(order = 2; order <= maxOrder; ++order)
    {
        // WICHTIG: Warten, bis die vorherige Ebene fertig geschrieben hat!
        //encoder->memoryBarrier(MTL::BarrierScopeBuffers);

        encoder->setBytes(&order, sizeof(order), 0, 2);
        encoder->dispatchThreads(grid, threadsPerThreadgroup);
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::reduceMaxError(MTL::ComputeCommandEncoder* encoder) {
    uint32_t count = mNatoms;

    // gpu layout
    auto width = mReduceMaxErrorPipeline->threadExecutionWidth();
    MTL::Size threadsPerThreadgroup(width, 1, 1);

    while(count > 1)
    {  
        encoder->setComputePipelineState(mReduceMaxErrorPipeline.get());

        encoder->setBuffer(mMaxError.buffer(), 0, 0);
        encoder->setBytes(&count, sizeof(count), 1);
 
        uint32_t outCount = (count + 1) / 2;
        MTL::Size grid(outCount, 1, 1);
        
        encoder->dispatchThreads( grid, threadsPerThreadgroup);

        // WICHTIG: Warten, bis die Reduktion im Buffer steht!
        //encoder->memoryBarrier(MTL::BarrierScopeBuffers);

        count = outCount;
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::findReinsertAtoms(MTL::ComputeCommandEncoder* encoder) 
{
    encoder->setComputePipelineState(mFindReinsertAtomsPipeline.get());

    encoder->setBuffer(mNextAtoms.metalBuffer(), 0, 0);
    encoder->setBuffer(mTree.branches()->metalBuffer(), 0, 1);
    encoder->setBuffer(mNeadReinsert.buffer(), 0, 2);
    encoder->setBuffer(mTree.leafs()->metalBuffer(), 0, 3);
    encoder->setBytes(&mBox, sizeof(mBox), 4);

    auto width = mFindReinsertAtomsPipeline->threadExecutionWidth();
    MTL::Size threadsPerThreadgroup(width, 1, 1);
    MTL::Size grid(mNatoms, 1, 1);

    encoder->dispatchThreads(grid, threadsPerThreadgroup);
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::reinsertAtoms()
{
    for(int i = 0; i < mNatoms; ++i)
    {
        if( mNeadReinsert[i] > 0)
        {
            auto atom   = *(mTree.leafs()->getPointer(i));
            auto rootId = atom.parentIndex();
            auto oct    = atom.possitonInParent();
            auto sub    = Tree::reference(&mTree, rootId);

            //printf("Reinsert atom %d\n", i);
            sub.deleate(oct);
            insertAtomInTree(mTree, mNextAtoms[i].value());

            auto size = sub.size();

            // if the branch is empty
            if (size == 0)
            {
                //printf("Deleate branch\n");
                while (size == 0) 
                {
                    auto parentOct = sub.self().possitonInParent();
                    sub.goUp();
     
                    if (sub) 
                    { 
                        sub.deleate(parentOct);
                        size = sub.size();
                    }
                    else
                    {
                        sub = mTree.root();
                        break;
                    } 
                }
                decrOrder(mTree, sub);  
            }
            //printf("done Atom %i\n", i);
        }
    }

    mNextAtoms = *(mTree.leafs());
}
//----------------------------------------------------------------------------------------------------------------------

/*
#define PROFILE_METAL_CALL(name, code) \
    commandBuffer = mQueue->commandBuffer(); \
    encoder = commandBuffer->computeCommandEncoder(); \
    start = std::chrono::steady_clock::now(); \
    code; \
    encoder->endEncoding(); \
    commandBuffer->commit(); \
    commandBuffer->waitUntilCompleted(); \
    end = std::chrono::steady_clock::now(); \
    time = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6; \
    printf("%-30s Time: %14.7lfs\n", name, time);
//----------------------------------------------------------------------------------------------------------------------
*/

void MetalGravity::step()
{
    auto* commandBuffer = mQueue->commandBuffer();
    auto* encoder       = commandBuffer->computeCommandEncoder();

    const int maxStepps = 100;
    float oldError      = 1e5f;

    //std::chrono::steady_clock::time_point start, end;
    //double time;

    //PROFILE_METAL_CALL("firstIntegral", firstIntegral(encoder);)
    firstIntegral(encoder);

    //PROFILE_METAL_CALL("updateCenterOfMass", updateCenterOfMass(encoder);)
    updateCenterOfMass(encoder);

    for (int i = 0; i < maxStepps; ++i)
    {
        //PROFILE_METAL_CALL("calculateAcelation", calculateAcelation(encoder, mNextAtoms.metalBuffer());)
        calculateAcelation(encoder, mNextAtoms.metalBuffer());
        //encoder->memoryBarrier(MTL::BarrierScopeBuffers);

        //PROFILE_METAL_CALL("secondIntegralAndError", secondIntegralAndError(encoder);)
        secondIntegralAndError(encoder);
        //encoder->memoryBarrier(MTL::BarrierScopeBuffers);

        //PROFILE_METAL_CALL("updateCenterOfMass", updateCenterOfMass(encoder);)
        updateCenterOfMass(encoder);
        //encoder->memoryBarrier(MTL::BarrierScopeBuffers);

        //PROFILE_METAL_CALL("reduceMaxError", reduceMaxError(encoder);)
        reduceMaxError(encoder);
        //encoder->memoryBarrier(MTL::BarrierScopeBuffers);
    
        encoder->endEncoding();
        commandBuffer->commit();

        //printf("comit\n");
        commandBuffer->waitUntilCompleted();

        auto maxError = mMaxError[0];

        //printf("Iteration %i:\tError: %11.7e\n", i, maxError);

        commandBuffer = mQueue->commandBuffer();
        encoder       = commandBuffer->computeCommandEncoder();

        if( maxError < 1e-5 || fabs(maxError - oldError) < 1e-5) break;

        oldError = maxError;
    }

    //PROFILE_METAL_CALL("integrateVelosseties", integrateVelosseties(encoder);)
    integrateVelosseties(encoder);
    //encoder->memoryBarrier(MTL::BarrierScopeBuffers);

    // atension, the tmporary buffer for the atoms ist mTree.leafs(), we swap it at the end back to mNextAtoms
    //PROFILE_METAL_CALL("findReinsertAtoms", findReinsertAtoms(encoder);)
    findReinsertAtoms(encoder);
   
    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    //encoder->release();
    //commandBuffer->release();

    std::swap(*(mTree.leafs()), mNextAtoms);

    //printf("Integraton done.\n");

    //start = std::chrono::steady_clock::now();
    reinsertAtoms();
    //end = std::chrono::steady_clock::now();
    //time = std::chrono::duration<double, std::micro>(end - start).count() * 1e-6;

    //printf("%-30s Time: %14.7lfs\n", "reinsertAtoms", time);
}
//----------------------------------------------------------------------------------------------------------------------

bool MetalGravity::printSnapshot(int step)
{
    char fname[512];
    char cwd[512];
    //snprintf(fname, 512, "%s/step_%09i.xyz", step);
    FILE* file = nullptr;

    if (getcwd(cwd, sizeof(cwd)) != nullptr) 
    {
        snprintf(fname, 512, "%s/step_%09i.xyz", cwd, step);

        file = fopen(fname, "w");
        if(file == nullptr) return false;
    } 
    else 
    {
        fprintf(stderr, "could not open file!\n");
        return false;
    }

    //FILE* file = fopen(fname, "w");
    //if(file == nullptr) return false;

    fprintf(file, "%i\n", mNatoms);
    fprintf(file, "Box: %14.7e\t%14.7e\t%14.7e\n", mBox.x, mBox.y, mBox.z);

    auto it = mTree.root().begin();
    auto end = mTree.root().end();

    while(it != end)
    {
        auto r = it->value().pos;
        auto v = it->value().vel;

        fprintf(file, "Xx\t%14.7e\t%14.7e\t%14.7e\t%14.7e\t%14.7e\t%14.7e\n", r.x, r.y, r.z, v.x, v.y, v.z);

        ++it;
    }

    fclose(file);
    return true;
}
//----------------------------------------------------------------------------------------------------------------------
