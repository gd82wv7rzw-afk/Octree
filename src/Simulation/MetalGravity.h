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
        using PipelinePtr = MetalStuff::Pipeline::ComputePipelinePtr;
        
        template< class T>
        using MetalBuffer = Octree::Implementation::MetalBuffer<T>;


    public:

        /// @brief the ctor
        /// @param device the metal device (the GPU)
        /// @param que    the commandque
        MetalGravity(MTL::Device* device, MTL::CommandQueue* que, int nAtoms, float freelength) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        /// @brief generate random possitions and veloseties. Set also the periodic box and the number of atoms
        /// @param box 
        /// @param natoms 
        void generateRandomConfiguration();
        //--------------------------------------------------------------------------------------------------------------

        /// @brief calculate the acceleration for every atom on the GPU
        void calculateAcelation(MTL::ComputeCommandEncoder* encoder, MTL::Buffer* atoms);
        //--------------------------------------------------------------------------------------------------------------

        /// @brief propagate the atoms in time temporaer
        void firstIntegral(MTL::ComputeCommandEncoder* encoder);
        void secondIntegralAndError(MTL::ComputeCommandEncoder* encoder);
        void integrateVelosseties(MTL::ComputeCommandEncoder* encode);
        //--------------------------------------------------------------------------------------------------------------

        void updateCenterOfMass(MTL::ComputeCommandEncoder* encoder);
        //--------------------------------------------------------------------------------------------------------------

        void reduceMaxError(MTL::ComputeCommandEncoder* enc);
        //--------------------------------------------------------------------------------------------------------------

        void step();
        //--------------------------------------------------------------------------------------------------------------

        auto* tree()       noexcept {return &mTree;}
        auto* tree() const noexcept {return &mTree;}
        //--------------------------------------------------------------------------------------------------------------

    private:

        void insertAtomInTree(Tree& tree, const Atom& atom);
        void updateOrder(Tree& tree, Tree::reference sub, int order);
        void buildPipelines();
        //--------------------------------------------------------------------------------------------------------------

        /// @brief the number of atoms
        int mNatoms{int(1e5)};
        //--------------------------------------------------------------------------------------------------------------

        /// @brief the bounding box, origin = (0, 0, 0)^T
        simd::float3 mBox{1.f, 1.f, 1.f};
        //--------------------------------------------------------------------------------------------------------------

        /// @brief the barnes hut criteria, if theta > mTheta => go in the node
        float mTheta{0.7f};
        //--------------------------------------------------------------------------------------------------------------

        float mDt{1e-2};
        //--------------------------------------------------------------------------------------------------------------

        Tree                  mTree;
        Tree::leafBuffer_type mNextAtoms;
        MetalArray<float>     mMaxError;
        std::mt19937          mRandom;
        MTL::Device*          mDevice{nullptr};
        MTL::CommandQueue*    mQueue{nullptr};
        std::vector<std::vector<uint32_t> > mOrderMap;
        //--------------------------------------------------------------------------------------------------------------

        PipelinePtr mCalculateAcelationPipeline{nullptr};
        PipelinePtr mFirstIntegralPipeline{nullptr};
        PipelinePtr mSecondIntegralPipeline{nullptr};
        PipelinePtr mUpdateCenterOfMassPipeline{nullptr};
        PipelinePtr mReduceMaxErrorPipeline{nullptr};
        PipelinePtr mIntegratteVelosetiesPipeline{nullptr};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------


} // namespace Simulation ----------------------------------------------------------------------------------------------