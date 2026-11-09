#include "TripleBuffer.h"

using namespace Rendering;

void TripleBuffer::allocate(MTL::Device* device, size_t count)
{
    for (auto& buffer : mBuffers) buffer.allocate(device, count);
    mRoles.store
    (
        encode
        (
            { 
                BufferIndex(0), 
                BufferIndex(1), 
                BufferIndex(2) 
            }, 
            false
        ),
        std::memory_order_relaxed
    );
}
//----------------------------------------------------------------------------------------------------------------------
 
uint32_t TripleBuffer::encode(const BufferRoles& roles, bool dirty)
{
    return roles.nextWrite
         | (roles.nextOutput << 2)
         | (roles.currentOutput << 4)
         | (static_cast<uint32_t>(dirty) << 6);
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
    auto roles = decode(mRoles.load(std::memory_order_relaxed));
    return &mBuffers[roles.nextWrite];
}
//----------------------------------------------------------------------------------------------------------------------

TripleBuffer::Buffer* TripleBuffer::getOutputBuffer()
{
    auto roles = decode(mRoles.load(std::memory_order_acquire));
    return &mBuffers[roles.currentOutput];
}
//----------------------------------------------------------------------------------------------------------------------

void TripleBuffer::publishInput()
{
    uint32_t expected = mRoles.load(std::memory_order_relaxed);

    while (true)
    {
        auto roles = decode(expected);

        BufferRoles next
        {
            roles.nextOutput,
            roles.nextWrite,
            roles.currentOutput
        };

        uint32_t desired = encode(next, true);

        if (
            mRoles.compare_exchange_weak
            (
                expected,
                desired,
                std::memory_order_release,
                std::memory_order_relaxed
            )
        )
        {
            return;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

void TripleBuffer::releaseOutput()
{
    uint32_t expected = mRoles.load(std::memory_order_relaxed);

    while (true)
    {
        auto roles = decode(expected);

        BufferRoles next
        {
            roles.currentOutput,
            roles.nextOutput,
            roles.nextWrite
        };

        uint32_t desired = encode(next);

        if (mRoles.compare_exchange_weak(
            expected,
            desired,
            std::memory_order_acq_rel,
            std::memory_order_relaxed))
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------

bool TripleBuffer::acquireOutput()
{
    uint32_t expected = mRoles.load(std::memory_order_acquire);

    if ((expected & (1u << 6)) == 0) return false;

    while (true)
    {
        if ((expected & (1u << 6)) == 0)
            return false;

        auto roles = decode(expected);

        BufferRoles next
        {
            roles.nextWrite,
            roles.currentOutput,
            roles.nextOutput
        };

        uint32_t desired = encode(next, false);

        if (mRoles.compare_exchange_weak(
            expected,
            desired,
            std::memory_order_acq_rel,
            std::memory_order_acquire))
            return true;
    }
}
//----------------------------------------------------------------------------------------------------------------------
