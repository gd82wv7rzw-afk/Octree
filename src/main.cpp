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
    MTL::CommandQueue* queue = device->newCommandQueue();
    branch_type branch;
    MetalBuffer<branch_type> buffer(device, queue, 1024);

    for(int i=0; i< 1024;++i)
    {
        buffer[i].value() = i;
    }
    
    auto b = buffer;


    for(int i= 0; i< 1024; ++ i){
        printf("%d\t %lf\n", i, b[i].value());
    }

    queue->release();
    device->release();

    return 0;
}