#pragma once

#include "Simulation/Atom.h"
#include "Simulation/Branch.h"
#include "Octree/Octree.h"

#include <random>
#include <Metal/Metal.hpp>

namespace Simulation {

class MetalGravity 
{
    private:

        using Tree = Octree::MetalTree<Simulation::Atom, Simulation::Branch>;
    public:

        MetalGravity(MTL::Device* device, MTL::CommandQueue* que) noexcept
            : mDevice{device}
            , mQueue{que}
            , mRandom{std::random_device{}()} 
            , mTree(Tree::branchBuffer_type(device, que), Tree::leafBuffer_type(device,que))
        {}
        //--------------------------------------------------------------------------------------------------------------

        void generateRandomConfiguration(const simd::float3& box, const int natoms);
        //--------------------------------------------------------------------------------------------------------------

        auto* tree()       noexcept {return &mTree;}
        auto* tree() const noexcept {return &mTree;}
        //--------------------------------------------------------------------------------------------------------------

    private:

        void insertAtomInTree(Tree& tree, const Atom& atom);
        void updateOrder(Tree& tree, Tree::reference sub, int order);
        //--------------------------------------------------------------------------------------------------------------

        int                 mNatoms{0};
        simd::float3        mBox{1.f, 1.f, 1.f};
        
        Tree                mTree;
        std::mt19937        mRandom;
        MTL::Device*        mDevice{nullptr};
        MTL::CommandQueue*  mQueue{nullptr};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------


} // namespace Simulation ----------------------------------------------------------------------------------------------