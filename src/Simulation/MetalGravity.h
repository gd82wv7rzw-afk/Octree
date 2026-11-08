#pragma once

#include "Simulation/Atom.h"
#include "Simulation/Branch.h"
#include "Octree/Octree.h"
#include "MetalStuff/MetalArray.hpp"
#include "MetalStuff/Pipeline/PipelineBuilder.h"
#include "Rendering/TripleBuffer.h"

#include <random>
#include <Metal/Metal.hpp>

namespace Gui 
{
    struct GuiParameter;
}

namespace Simulation {

class MetalGravity 
{
    private:

        using Tree = Octree::MetalTree<Simulation::Atom, Simulation::Branch>;
        using id_type = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::id_type;
        using PipelinePtr = MetalStuff::Pipeline::ComputePipelinePtr;
        using TreeBranch = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::branch_type;
        using TreeLeaf   = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::leaf_type;
        //-------------------------------------------------------------------------------------------------------------- 

    public:

        /// @brief the ctor
        /// @param device the metal device (the GPU)
        /// @param que    the commandque
        MetalGravity(MTL::Device* device, MTL::CommandQueue* que) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        void generateRandomConfiguration(int nAtoms, float freelength);
        bool readFile(const char* filename);
        //--------------------------------------------------------------------------------------------------------------

        void start(Gui::GuiParameter* parameters) { mIsRunning = true; printf("run\n"); run(parameters); }
        void stop()  { mIsRunning = false; }
        void run(Gui::GuiParameter* parameters);
        //--------------------------------------------------------------------------------------------------------------
        
        void step();
        //--------------------------------------------------------------------------------------------------------------

        float dt() const {return mDt;}
        //--------------------------------------------------------------------------------------------------------------

        bool printSnapshot(const std::string& path, int step);
        //--------------------------------------------------------------------------------------------------------------

        auto* getOutput() {return mIputOutputBuf.getOutputBuffer(); }
        //--------------------------------------------------------------------------------------------------------------

        // call it after the rendering
        void renderingIsDone() { mIputOutputBuf.releaseOutput();}
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
        void integrateVelosseties(MTL::ComputeCommandEncoder* encode, float temperature);
        void updateCenterOfMass(MTL::ComputeCommandEncoder* encoder);
        void reduceMaxError(MTL::ComputeCommandEncoder* encoder);
        void findReinsertAtoms(MTL::ComputeCommandEncoder* encoder);
        void copyOutput(MTL::ComputeCommandEncoder* encoder);
        void reinsertAtoms();
        //--------------------------------------------------------------------------------------------------------------

        std::atomic_bool mIsRunning{false};
        Rendering::TripleBuffer mIputOutputBuf;
        Gui::GuiParameter* mParameters{nullptr};
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

        float mDt{0.1f};
        int mIterationSteps{0};
        int mCounter{0};
        int mStep{0};
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
        PipelinePtr mCopyOutpuPipeline{nullptr};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------


} // namespace Simulation ----------------------------------------------------------------------------------------------