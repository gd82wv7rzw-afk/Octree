#pragma once

#include "Octree/Octree.h"
#include "Simulation/Atom.h"
#include "Simulation/Branch.h"

#include <simd/simd.h>

namespace Simulation {

class CpuForce
{
    public:

        using TreeBranch = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::branch_type;
        using TreeLeaf   = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::leaf_type;
        using Tree = Octree::MetalTree<Simulation::Atom, Simulation::Branch>;
        using id_type = Octree::Implementation::MetalTree<Simulation::Atom, Simulation::Branch>::id_type;
        using atomArry = Octree::Implementation::MetalBuffer<TreeLeaf>;

        void calculateAcceleration(atomArry& atoms, Tree& tree, int nAtoms, const simd::float3& box, float theta);
        void calculateAcceleration(Simulation::Atom* atom, Tree& tree, const simd::float3& box, float theta, int atomid);
};


} // namespace Simulation