#pragma once

#include "Simulation/Atom.h"
#include "Simulation/Branch.h"
#include "Octree/Octree.h"
#include "MetalStuff/MetalArray.hpp"

#include <random>
#include <Metal/Metal.hpp>

namespace Simulation {

class MetalGravity 
{
    private:

        using Tree = Octree::MetalTree<Simulation::Atom, Simulation::Branch>;
        using id_type = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::id_type;
        using PipelinePtr = MetalStuff::Pipeline::ComputePipelinePtr;
        using TreeBranch = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::branch_type;
        
        template< class T>
        using MetalBuffer = Octree::Implementation::MetalBuffer<T>;

    public:

        /// @brief the ctor
        /// @param device the metal device (the GPU)
        /// @param que    the commandque
        MetalGravity(MTL::Device* device, MTL::CommandQueue* que, int nAtoms, float freelength) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        void generateRandomConfiguration();
        //--------------------------------------------------------------------------------------------------------------
        
        void step();
        //--------------------------------------------------------------------------------------------------------------

        bool printSnapshot(int step);
        //--------------------------------------------------------------------------------------------------------------

        auto* tree()       noexcept {return &mTree;}
        auto* tree() const noexcept {return &mTree;}
        //--------------------------------------------------------------------------------------------------------------

    private:

        Tree::reference::iterator insertAtomInTree(Tree& tree, const Atom& atom);
        void incrOrder(Tree& tree, Tree::reference sub);
        void decrOrder(Tree& tree, Tree::reference sub);
        void buildPipelines();

        void calculateAcelation(MTL::ComputeCommandEncoder* encoder, MTL::Buffer* atoms);
        void firstIntegral(MTL::ComputeCommandEncoder* encoder);
        void secondIntegralAndError(MTL::ComputeCommandEncoder* encoder);
        void integrateVelosseties(MTL::ComputeCommandEncoder* encode);
        void updateCenterOfMass(MTL::ComputeCommandEncoder* encoder);
        void reduceMaxError(MTL::ComputeCommandEncoder* encoder);
        void findReinsertAtoms(MTL::ComputeCommandEncoder* encoder);
        void reinsertAtoms();
        //--------------------------------------------------------------------------------------------------------------

        /// @brief the number of atoms
        int mNatoms{int(1e5)};
        //--------------------------------------------------------------------------------------------------------------

        /// @brief the bounding box, origin = (0, 0, 0)^T
        simd::float3 mBox{1.f, 1.f, 1.f};
        //--------------------------------------------------------------------------------------------------------------

        /// @brief the barnes hut criteria, if theta > mTheta => go in the node
        float mTheta{0.8f};
        //--------------------------------------------------------------------------------------------------------------

        float mDt{2.5e-1};
        //--------------------------------------------------------------------------------------------------------------

        Tree                  mTree;
        Tree::leafBuffer_type mNextAtoms;
        MetalArray<float>     mMaxError;
        MetalArray<int>       mNeadReinsert;
        std::mt19937          mRandom;
        MTL::Device*          mDevice{nullptr};
        MTL::CommandQueue*    mQueue{nullptr};
        //--------------------------------------------------------------------------------------------------------------

        PipelinePtr mCalculateAcelationPipeline{nullptr};
        PipelinePtr mFirstIntegralPipeline{nullptr};
        PipelinePtr mSecondIntegralPipeline{nullptr};
        PipelinePtr mUpdateCenterOfMassPipeline{nullptr};
        PipelinePtr mReduceMaxErrorPipeline{nullptr};
        PipelinePtr mIntegratteVelosetiesPipeline{nullptr};
        PipelinePtr mFindReinsertAtomsPipeline{nullptr};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------


} // namespace Simulation ----------------------------------------------------------------------------------------------