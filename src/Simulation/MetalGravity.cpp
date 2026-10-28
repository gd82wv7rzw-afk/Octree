#include "MetalGravity.h"

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
            float x = std::normal_distribution<float>(0.f, mBox.x / 2)(mRandom);
            float y = std::normal_distribution<float>(0.f, mBox.y / 2)(mRandom);
            float z = std::normal_distribution<float>(0.f, mBox.z / 2)(mRandom);

            float3 p{x, y, z};

            if (dot(p,p) < dot(mBox, mBox)/4) 
            {
                point = p;
                break;
            }
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

    Tree tmp(Tree::branchBuffer_type(mDevice, mQueue), Tree::leafBuffer_type(mDevice, mQueue));
    tmp.leafs()->allocate(mNatoms);
    tmp.branches()->allocate(mTree.numberOfBranches());

    for(auto a: mTree.root())
    {
        insertAtomInTree(tmp, a.value());
    }
    std::swap(mTree, tmp);

    //buildOrderMap();
    mNextAtoms = *(mTree.leafs());

    //accelaeration
    auto* commandBuffer = mQueue->commandBuffer();
    auto* encoder       = commandBuffer->computeCommandEncoder();

    calculateAcelation(encoder, mTree.leafs()->metalBuffer());

    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    encoder->release();
    commandBuffer->release();
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::insertAtomInTree(Tree& tree, const Atom& atom)
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
        sub.self().value().width = width * 2;
        sub.self().value().order = order;

        return;
    }

    // update the center of mass
    {
        auto sz = sub.size();
        sub.self().value().com = (sub.self().value().com * sz + atom.pos) / (sz + 1);
    }

    if (sub.isEmpty(oct)) 
    {
        auto leaf = sub.newLeaf(oct);
        leaf->value() = atom;
        
        return;
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

            auto sz = sub.size(oct);

            sub.goIn(oct);
            sub.self().value().com   = (tmp.com * sz + atom.pos) / (sz + 1);
            //sub.self().value().width = width;

            oct = (atom.pos.x >= center.x) | (atom.pos.y >= center.y) << 1 | (atom.pos.z >= center.z) << 2;
        }

        if (sub.isLeaf(oct)) 
        {
            auto leaf = sub.leaf(oct);
            sub.deleate(oct);
            sub.newSubTree(oct);

            sub.goIn(oct);
            sub.self().value().com   = (leaf.value().pos + atom.pos) * 0.5f;
            //sub.self().value().order = 1;

            offset.x = oct & 0b001 ? offset.x + width: offset.x;
            offset.y = oct & 0b010 ? offset.y + width: offset.y;
            offset.z = oct & 0b100 ? offset.z + width: offset.z;

            width /= 2;
            sub.self().value().width = width * 2;

            center = offset + float3{width, width, width};
           
            oct = oct = (atom.pos.x >= center.x) | (atom.pos.y >= atom.pos.y) << 1 | (atom.pos.z >= center.z) << 2;
 
            sub.newLeaf(oct);
            sub.leaf(oct).value() = atom;

            oct = ~oct & 7;
 
            sub.newLeaf(oct);
            sub.leaf(oct).value() = leaf.value();

            updateOrder(mTree, sub, 1);

            return;
        }           
        else
        {
            auto leaf = sub.newLeaf(oct);
            leaf->value() = atom;
            
            return;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::updateOrder(Tree& tree, Tree::reference sub, int order)
{
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

    // gpu layout
    const auto width = mCalculateAcelationPipeline->threadExecutionWidth();
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
        encoder->setBytes(&order, sizeof(order), 0, 1);
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
        count = outCount;
    }
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::step()
{
    auto* commandBuffer = mQueue->commandBuffer();
    auto* encoder       = commandBuffer->computeCommandEncoder();
    const int maxStepps = 100;
    float oldError      = 1.f;

    firstIntegral(encoder);
    updateCenterOfMass(encoder);

    for (int i = 0; i < maxStepps; ++i)
    {
        calculateAcelation(encoder, mNextAtoms.metalBuffer());
        secondIntegralAndError(encoder);
        updateCenterOfMass(encoder);
        reduceMaxError(encoder);
        
        encoder->endEncoding();
        commandBuffer->commit();
        commandBuffer->waitUntilCompleted();

        auto maxError = mMaxError[0];

        printf("Iteration %i:\tError: %11.7e\n", i, maxError);

        commandBuffer = mQueue->commandBuffer();
        encoder       = commandBuffer->computeCommandEncoder();

        if( maxError < 1e-5 || fabs(maxError - oldError) < 1e-5) break;

        oldError = maxError;
    }

    integrateVelosseties(encoder);
   
    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    encoder->release();
    commandBuffer->release();

    std::swap(*(mTree.leafs()), mNextAtoms);
}
//----------------------------------------------------------------------------------------------------------------------
