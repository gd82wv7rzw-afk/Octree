#include "TripleBuffer.h"

using namespace Rendering;

void TripleBuffer::allocate(MTL::Device* device, size_t count)
{
    for (auto& buffer : mBuffers) buffer.allocate(device, count);
    test.allocate(device, count);
}
//----------------------------------------------------------------------------------------------------------------------
 
uint32_t TripleBuffer::encode(const BufferRoles& roles)
{
    return roles.nextWrite
         | (roles.nextOutput << 2)
         | (roles.currentOutput << 4);
}
//----------------------------------------------------------------------------------------------------------------------

TripleBuffer::BufferRoles TripleBuffer::decode(uint32_t value)
{
    return {
        static_cast<BufferIndex>( value       & 0x3),
        static_cast<BufferIndex>((value >> 2) & 0x3),
        static_cast<BufferIndex>((value >> 4) & 0x3)
    };
}
//----------------------------------------------------------------------------------------------------------------------

TripleBuffer::Buffer* TripleBuffer::getInputBuffer()
{
    auto roles = decode(mRoles);
    //return &test;
    return &mBuffers[roles.nextWrite];
}
//----------------------------------------------------------------------------------------------------------------------

TripleBuffer::Buffer* TripleBuffer::getOutputBuffer()
{
    auto roles = decode(mRoles);
    return &mBuffers[roles.nextOutput];
}
//----------------------------------------------------------------------------------------------------------------------

void TripleBuffer::publishInput()
{
    auto roles = decode(mRoles);

    BufferRoles next
    {
        roles.nextOutput,
        roles.nextWrite,
        roles.currentOutput
    };

    mRoles = encode(next);
}
//----------------------------------------------------------------------------------------------------------------------

void TripleBuffer::releaseOutput()
{
    auto roles = decode(mRoles);

    BufferRoles next{
        roles.currentOutput,
        roles.nextOutput,
        roles.nextWrite
    };

    mRoles = encode(next);
}
//----------------------------------------------------------------------------------------------------------------------
