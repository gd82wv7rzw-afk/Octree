#pragma once

#include "Octree/Octree.h"
#include <random>
#include <simd/simd.h>
#include <simd/vector_types.h>
#include <vector>

struct Branch
{
    simd::float3 com{0};
    float        with{1};
};
//----------------------------------------------------------------------------------------------------------------------

struct Atom
{
    simd::float3 pos{0};
    simd::float3 vel{0};
    simd::float3 a{0};
    simd::float3 aNaiv{0};
    int oldIdx{-1};
};
//----------------------------------------------------------------------------------------------------------------------

class SimulatorV0
{
    public:
        using Tree = Octree::Tree<Atom, Branch>;
        //--------------------------------------------------------------------------------------------------------------

        SimulatorV0();
        //--------------------------------------------------------------------------------------------------------------

        void init();
        //--------------------------------------------------------------------------------------------------------------

        simd::float3 grad(const simd::float3& x);
        //--------------------------------------------------------------------------------------------------------------

        // b - a, periodisch
        simd::float3 calculateDistance(const simd::float3& a, const simd::float3& b);
        //--------------------------------------------------------------------------------------------------------------

        float maxDim(const simd::float3& x);
        //--------------------------------------------------------------------------------------------------------------

        void calculateAcceleration(Tree::reference::iterator it, Tree& tree);
        void calculateAcceleration(Tree& tree);
        //--------------------------------------------------------------------------------------------------------------

        void calculateAccelerationNaive(Tree::reference::iterator it);
        void calculateAccelerationNaive();
        //--------------------------------------------------------------------------------------------------------------

        void printAndCalcError();
        //--------------------------------------------------------------------------------------------------------------

        void printSnapShot();
        //--------------------------------------------------------------------------------------------------------------

        void firstIntegrate( Tree::reference::iterator it);
        float secondIntegrateAndError(Tree::reference::iterator it);
        void integrateVeloseties(Tree::reference::iterator it);
        void step();
        void integrate();
        void aniling(Tree::reference::iterator it);
        void aniling();
        //--------------------------------------------------------------------------------------------------------------

        void updateTree(Tree& tree);
        //--------------------------------------------------------------------------------------------------------------

    //private:

        Tree mTree;
        Tree mNextTree;
        std::vector<Atom> mTmpAtoms;

        int  mNAtoms{int(1e5)};
        std::mt19937 mRng;
        
        float mFreeLength{10000};
        float mLength{1};

        float mDt{5e-3};
       
        float mTheta{0.5};

        float mT{1.0};

        float mR0{0};
        float mR02{0};
        float mR013{0};
        
        float mRoh{1.f / mFreeLength / mFreeLength / mFreeLength};

};
//----------------------------------------------------------------------------------------------------------------------

