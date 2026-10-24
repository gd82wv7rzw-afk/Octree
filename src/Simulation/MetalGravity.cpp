#include "MetalGravity.h"

using namespace Simulation;
using namespace simd;
using namespace std;

void MetalGravity::generateRandomConfiguration(const simd::float3& box, const int natoms)
{
    mBox    = box;
    mNatoms = natoms;

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
}
//----------------------------------------------------------------------------------------------------------------------

void MetalGravity::insertAtomInTree(Tree& tree, const Atom& atom)
{
    float3 center{0.0f, 0.0f, 0.0f};
    float3 offset{-0.5f*mBox.x, -0.5f*mBox.y, -0.5f*mBox.z}; 

    char   oct   = (atom.pos.x >= center.x) | (atom.pos.y >= center.y) << 1 | (atom.pos.z >= center.z) << 2;
    float  width = 0.5f; 
    int    order = 1;

    auto sub = tree.root();

    // only the first time
    if (tree.numberOfBranches() == 0) 
    {
        auto leaf = sub.newLeaf(oct);
        leaf->value() = atom;

        sub.self().value().com   = atom.pos;
        sub.self().value().width = width;
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
            sub.self().value().width = width;

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
            sub.self().value().width = width;

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
        sub.self().value().order = order;
        sub.goUp();
        ++order;
    }
}
//----------------------------------------------------------------------------------------------------------------------