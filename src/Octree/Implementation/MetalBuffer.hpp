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
        //--------------------------------------------------------------------------------------------------------------

        MetalBuffer(MTL::Device* device, MTL::CommandQueue* queue, size_type size);
        MetalBuffer(const MetalBuffer& other);
        MetalBuffer(MetalBuffer&& other);
        MetalBuffer& operator=(const MetalBuffer& other);
        MetalBuffer& operator=(MetalBuffer&& other);
        ~MetalBuffer();
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Convert the MTL::Buffer to the pointer T*
        /// @return the pointer T*
        inline pointer       data()       noexcept { return reinterpret_cast<pointer>( mBuffer->contents()); }
        inline const_pointer data() const noexcept { return reinterpret_cast<pointer>( mBuffer->contents()); }
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Returns a pointer to the element at the specified index.
        /// @param i Index of the element.
        /// @return Pointer to the element, or nullptr if the index is invalid or the element has been deleted.
        constexpr pointer getPointer(size_type i) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Returns a pointer to the element at the specified index.
        /// @param i Index of the element.
        /// @return Pointer to the element, or nullptr if the index is invalid or the element has been deleted.
        constexpr const_pointer getPointer(size_type i) const noexcept;
        //--------------------------------------------------------------------------------------------------------------

        reference       operator[](size_type pos)       noexcept {return *(getPointer(pos));}
        const_reference operator[](size_type pos) const noexcept {return *(getPointer(pos));}
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Returns the next available insertion index.
        /// If the free list is empty, the container size is returned.
        /// Otherwise, the index of the first reusable entry is returned.
        /// @return Index of the next insertion position.
        constexpr size_type nextIndex() const noexcept  { return mNextIndex == id_type::IndexMask ? mSize : mNextIndex;}
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Inserts an element into the buffer.
        /// Appends the element if no deleted entries are available; otherwise,
        /// reuses the first entry in the free list.
        /// @param x Element to insert.
        void push_back(const_reference x) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        /// @brief Marks an element as deleted and adds it to the free list.
        /// @param i Index of the element to delete.
        constexpr void deleate(size_type i) noexcept;
        //--------------------------------------------------------------------------------------------------------------

    private:

        void buildPipelines();
        void copyFrom(MTL::Buffer* other);

        void allocate(size_type size);
        //--------------------------------------------------------------------------------------------------------------

        MTL::Buffer* mBuffer{nullptr};
        MTL::Device* mDevice{nullptr};
        MTL::CommandQueue* mQueue{nullptr};
        size_type    mSize{0};
        
        /// @brief Index of the first deleted entry, or IndexMask if none exist.
        size_type mNextIndex{id_type::IndexMask};

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
MetalBuffer<T>::MetalBuffer(const MetalBuffer& other)
    : mDevice(other.mDevice)
    , mQueue(other.mQueue)
    , mSize(other.mSize)
    , mNextIndex(other.mNextIndex)
{
    mBuffer = mDevice->newBuffer(mSize * sizeof(T), MTL::ResourceStorageModeShared);
    buildPipelines();
    copyFrom(other.mBuffer);
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>&
MetalBuffer<T>::operator=(const MetalBuffer& other)
{
    assert(this != &other);
    MetalBuffer tmp(other);

    mBuffer->release();

    mSize   = 0;
    mDevice = nullptr;
    mQueue  = nullptr;
    mBuffer = nullptr;
    
    std::swap(*this = tmp);

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>&
MetalBuffer<T>::operator=(MetalBuffer&& other)
{
    assert(this != &other);

    if (mBuffer) mBuffer->release();

    mSize          = other.mSize;
    mDevice        = other.mDevice;
    mQueue         = other.mQueue;
    mBuffer        = other.mBuffer;
    mNextIndex     = other.mNextIndex;
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
    , mNextIndex(other.mNextIndex)
    , mCopyPipeline(std::move(other.mCopyPipeline))
{
    other.mDevice = nullptr;
    other.mQueue  = nullptr;
    other.mBuffer = nullptr;
    other.mSize   = 0;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>::~MetalBuffer()
{
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
MetalBuffer<T>::copyFrom(MTL::Buffer* other)
{
    assert(other != nullptr); 

    const auto byteCount = mSize * sizeof(T);
    if (byteCount == 0) return;

    auto* commandBuffer = mQueue->commandBuffer();
    auto* encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(mCopyPipeline.get());
    encoder->setBuffer(other, 0, 0);
    encoder->setBuffer(mBuffer,       0, 1);

    const auto size = static_cast<uint32_t>(byteCount);
    encoder->setBytes(&size, sizeof(size), 2);

    const auto width = mCopyPipeline->threadExecutionWidth();

    encoder->dispatchThreads(
        MTL::Size(byteCount, 1, 1),
        MTL::Size(width, 1, 1));

    encoder->endEncoding();

    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}
//----------------------------------------------------------------------------------------------------------------------

template<class T>
constexpr typename MetalBuffer<T>::pointer
MetalBuffer<T>::getPointer(size_type i) noexcept
{
    if (!(i < mSize)) 
    {
        return nullptr;
    }

    auto ret = data() + i;

    size_type idx = ret->parendId();

    if (idx & id_type::DeletedBit) 
    {
        return nullptr;
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------

template<class T>
constexpr typename MetalBuffer<T>::const_pointer
MetalBuffer<T>::getPointer(size_type i) const noexcept
{
    if (!(i < mSize)) 
    {
        return nullptr;
    }

    auto ret = data() + i;

    size_type idx = ret->parendId();

    if (idx & id_type::DeletedBit) 
    {
        return nullptr;
    }

    return ret;
}
//----------------------------------------------------------------------------------------------------------------------

template<class T>
void
MetalBuffer<T>::allocate(size_type size)
{
    assert(size > mSize);

    auto tmp = mDevice->newBuffer(size * sizeof(T), MTL::ResourceStorageModeShared);

    std::swap(mBuffer, tmp);

    copyFrom(tmp);

    tmp->release();

    for (int i = mSize; i < size; ++i)
    {
        getPointer(i)->parendId() = mNextIndex | id_type::DeletedBit;
        mNextIndex = i;
    }

    mSize = size;
}
//----------------------------------------------------------------------------------------------------------------------

template<class T>
void
MetalBuffer<T>::push_back(const_reference x) noexcept
{
    if (mNextIndex == id_type::IndexMask) 
    {
        allocate(mSize + 1);
    }
        
    auto* p = getPointer(mNextIndex);

    auto idx = p->parendId() & id_type::IndexMask;
    mNextIndex = idx;
    *p = x;
}
//----------------------------------------------------------------------------------------------------------------------

template<class T>
constexpr void
MetalBuffer<T>::deleate(size_type i) noexcept
{
    auto* p = getPointer(i);

    if (p == nullptr) 
    {
        return;
    }

    auto idx = mNextIndex;
    mNextIndex = i;

    p->parendId() = id_type::DeletedBit | idx;
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace Implementation ------------------------------------------------------------------------------------------