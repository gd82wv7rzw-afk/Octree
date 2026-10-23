#pragma once

#include "Octree/Implementation/TreeID.hpp"
#include "MetalStuff/Pipeline/PipelineBuilder.h"

#include <Metal/Metal.hpp>
#include <atomic>
#include <cassert>


namespace Octree::Implementation {

template <class T>
class MetalBuffer
{
    private:

        using id_type        = TreeID<typename T::size_type>;
        //--------------------------------------------------------------------------------------------------------------

    public:

        using size_type       = typename id_type::size_type;

        using value_type      = T;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;


        using iterator        = T*;
        using const_iterator  = const T*;
        //--------------------------------------------------------------------------------------------------------------

        MetalBuffer(MTL::Device* device, MTL::CommandQueue* queue, size_type size);
        MetalBuffer(const MetalBuffer& other);
        MetalBuffer(MetalBuffer&& other);
        MetalBuffer& operator=(const MetalBuffer& other);
        MetalBuffer& operator=(MetalBuffer&& other);
        ~MetalBuffer();
        //--------------------------------------------------------------------------------------------------------------

        bool isBusy() const { return mIsBusy.load(); }
        //--------------------------------------------------------------------------------------------------------------

        inline pointer       data()       noexcept { return reinterpret_cast<pointer>( mBuffer->contents()); }
        inline const_pointer data() const noexcept { return reinterpret_cast<pointer>( mBuffer->contents()); }
        //--------------------------------------------------------------------------------------------------------------

        reference       operator[](size_type pos)       noexcept {return *(data() +pos);}
        const_reference operator[](size_type pos) const noexcept {return *(data() +pos);}
        //--------------------------------------------------------------------------------------------------------------

    private:

        void buildPipelines();
        void copyFrom(const MetalBuffer& other);
        //--------------------------------------------------------------------------------------------------------------

        MTL::Buffer* mBuffer{nullptr};
        MTL::Device* mDevice{nullptr};
        MTL::CommandQueue* mQueue{nullptr};
        size_type    mSize{0};

        std::atomic_bool mIsBusy{false};
        MetalStuff::Pipeline::ComputePipelinePtr mCopyPipeline{nullptr}; 
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

// Implementation ------------------------------------------------------------------------------------------------------
template <class T>
MetalBuffer<T>::MetalBuffer(MTL::Device* device, MTL::CommandQueue* queue, size_type size)
    : mDevice(device), mQueue(queue), mSize(size)
{
    mBuffer = mDevice->newBuffer(mSize * sizeof(T), MTL::ResourceStorageModeShared);

    buildPipelines();
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>::MetalBuffer(const MetalBuffer& other): MetalBuffer(other.mDevice, other.mQueue, other.mSize)
{
    copyFrom(other);
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>&
MetalBuffer<T>::operator=(const MetalBuffer& other)
{
    assert(this != &other);
    assert(mSize == other.mSize);

    copyFrom(other);
    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>&
MetalBuffer<T>::operator=(MetalBuffer&& other)
{
    assert(this != &other);
    assert(!mIsBusy.load());
    assert(!other.mIsBusy.load());

    if (mBuffer) mBuffer->release();

    mSize          = other.mSize;
    mDevice        = other.mDevice;
    mQueue         = other.mQueue;
    mBuffer        = other.mBuffer;
    mCopyPipeline  = std::move(other.mCopyPipeline);

    other.mSize   = 0;
    other.mDevice = nullptr;
    other.mQueue  = nullptr;
    other.mBuffer = nullptr;

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>::MetalBuffer(MetalBuffer&& other)
    : mBuffer(other.mBuffer)
    , mDevice(other.mDevice)
    , mQueue(other.mQueue)
    , mSize(other.mSize)
    , mCopyPipeline(std::move(other.mCopyPipeline))
{
    assert(!other.mIsBusy.load());

    other.mDevice = nullptr;
    other.mQueue  = nullptr;
    other.mBuffer = nullptr;
    other.mSize   = 0;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>::~MetalBuffer()
{
    assert(!mIsBusy.load());

    if (mBuffer) 
    {
        mBuffer->release();
        mBuffer = nullptr;
    }

    mSize   = 0;
    mDevice = nullptr;
    mQueue  = nullptr;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
void 
MetalBuffer<T>::buildPipelines()
{
    PipelineBuilder builder(mDevice);
    mCopyPipeline = builder.buildCompute("/default.metallib", "copy");
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
void 
MetalBuffer<T>::copyFrom(const MetalBuffer& other)
{
    assert(mSize == other.mSize);
    assert(!mIsBusy.load());
    assert(!other.mIsBusy.load());

    const auto byteCount = mSize * sizeof(T);
    if (byteCount == 0) return;

    auto* commandBuffer = mQueue->commandBuffer();
    auto* encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(mCopyPipeline.get());
    encoder->setBuffer(other.mBuffer, 0, 0);
    encoder->setBuffer(mBuffer,       0, 1);

    const auto size = static_cast<uint32_t>(byteCount);
    encoder->setBytes(&size, sizeof(size), 2);

    const auto width = mCopyPipeline->threadExecutionWidth();

    encoder->dispatchThreads(
        MTL::Size(byteCount, 1, 1),
        MTL::Size(width, 1, 1));

    encoder->endEncoding();

    mIsBusy.store(true);

    commandBuffer->addCompletedHandler(^(MTL::CommandBuffer* completed)
    {
        mIsBusy.store(false);
    });

    commandBuffer->commit();
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace Implementation ------------------------------------------------------------------------------------------