#include "Octree/Implementation/MetalBuffer.hpp"
#include "Octree/Implementation/Nodes/BaseBranch.hpp"

#include <Metal/Metal.hpp>
#include <Foundation/Foundation.hpp>


using namespace std;
using namespace Octree;
using namespace Implementation;
using branch_type = Nodes::BaseBranch<double, TreeID<int> >;

int main(int argc, char** argv)
{
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    branch_type branch;
    MetalBuffer<branch_type> buffer(device, 1024);

    device->release();

    return 0;
}