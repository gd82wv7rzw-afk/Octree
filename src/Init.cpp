#include "Init.h"
#include <random>
#include <simd/vector_types.h>

using namespace simd;

Tree init()
{
    std::mt19937 rng(std::random_device{}());
    static const int NParticle = 1e6;
    Tree tree;

    auto sub = *(tree.begin());
    float3 center{0};

    for (int i = 0; i < NParticle; ++i) 
    {
        float3 point
        {
            std::uniform_real_distribution<float>(-0.5f, 0.5f)(rng),
            std::uniform_real_distribution<float>(-0.5f, 0.5f)(rng),
            std::uniform_real_distribution<float>(-0.5f, 0.5f)(rng)
        };  

        center = {0,0,0};
        // the target octant
        char oct = (point.x >= center.x) | (point.y >= center.y) << 1 | (point.z >= center.z) << 2;

        float with = 0.5;
        float3 offset{-with, -with, -with};

        sub = tree.root();

        // only the first time
        if (sub == nullptr) 
        {
            sub.newLeaf(oct);
            sub.leaf(oct).value()  = point;

            sub.self().value().com  = point;
            sub.self().value().with = with;

            continue;
        }

        // update the center of mass
        {
            auto sz = sub.size();
            sub.self().value().com = (sub.self().value().com * sz + point) / (sz + 1);
        }

        if (sub.isEmpty(oct)) 
        {
            auto leaf = sub.newLeaf(oct);
            leaf->value() = point;
            continue;
        }
        else 
        {
            while (sub.isBranch(oct)) 
            {
                //sub.goIn(oct);
                auto tmp = sub.branch(oct).value();

                offset.x = oct & 0b001 ? offset.x + with: offset.x;
                offset.y = oct & 0b010 ? offset.y + with: offset.y;
                offset.z = oct & 0b100 ? offset.z + with: offset.z;

                with /= 2;

                center = offset + float3{with, with, with};

                auto sz = sub.size(oct);

                sub.goIn(oct);
                sub.self().value().com  = (tmp.com * sz + point) / (sz + 1);
                sub.self().value().with = with;

                oct = (point.x >= center.x) | (point.y >= center.y) << 1 | (point.z >= center.z) << 2;
            }

            if (sub.isLeaf(oct)) 
            {
                auto leaf = sub.leaf(oct);
                sub.deleate(oct);
                sub.newSubTree(oct);

                sub.goIn(oct);
                sub.self().value().com  = (leaf.value() + point) * 0.5f;

                offset.x = oct & 0b001 ? offset.x + with: offset.x;
                offset.y = oct & 0b010 ? offset.y + with: offset.y;
                offset.z = oct & 0b100 ? offset.z + with: offset.z;

                with /= 2;
                sub.self().value().with = with;

                center = offset + float3{with, with, with};
               
                oct = oct = (point.x >= center.x) | (point.y >= center.y) << 1 | (point.z >= center.z) << 2;
 
                sub.newLeaf(oct);
                sub.leaf(oct).value() = point;

                oct = ~oct & 7;
 
                sub.newLeaf(oct);
                sub.leaf(oct).value() = leaf.value();

                continue;
            }           
            else
            {
                auto leaf = sub.newLeaf(oct);
                leaf->value() = point;
                continue;
            }
        }
    }

    return tree;
}