#pragma once

#include "Octree/Implementation/TreeID.hpp"

#include <Metal/Metal.hpp>


namespace Octree::Implementation {

template <class T>
class MetalBuffer
{
    private:

        using id_type        = TreeID<typename T::size_type>;
        //--------------------------------------------------------------------------------------------------------------
    
    public:

        using size_type       = id_type::size_type;

        using value_type      = T;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;
        
    
        using iterator        = T*;
        using const_iterator  = const T*;
        //--------------------------------------------------------------------------------------------------------------

        MetalBuffer(MTL::Device* device, size_type size);
        MetalBuffer(const MetalBuffer& other);
        MetalBuffer(MetalBuffer&& other);
        MetalBuffer& operator=(const MetalBuffer& other);
        MetalBuffer& operator=(MetalBuffer&& other);
        ~MetalBuffer();
        //--------------------------------------------------------------------------------------------------------------

    private:

        MTL::Buffer* mBuffer{nullptr};
        MTL::Device* mDevice{nullptr};
        size_type    mSize{0};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

// Implementation ------------------------------------------------------------------------------------------------------
template <class T>
MetalBuffer<T>::MetalBuffer(MTL::Device* device, size_type size): mDevice(device), mSize(size)
{
    mBuffer = mDevice->newBuffer(mSize * sizeof(T), MTL::ResourceStorageModeShared);
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>::MetalBuffer(const MetalBuffer& other): MetalBuffer(other.mDevice, other.mSize)
{
    // TODO: copy data from other buffer
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>&
MetalBuffer<T>::operator=(const MetalBuffer& other)
{
    assert (this != &other);

    // TODO: copy data from other buffer
    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>&
MetalBuffer<T>::operator=(MetalBuffer&& other)
{
    assert(this != &other);

    if (mBuffer) mBuffer->release();

    std::move(other.mSize, mSize);
    std::move(other.mDevice, mDevice);
    std::move(other.mBuffer, mBuffer);

    other.mSize   = 0;
    other.mDevice = nullptr;
    other.mBuffer = nullptr;

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template <class T>
MetalBuffer<T>::MetalBuffer(MetalBuffer&& other)
    : mDevice(other.mDevice)
    , mBuffer(other.mBuffer)
    , mSize(other.mSize)
{
    other.mDevice = nullptr;
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
}
//----------------------------------------------------------------------------------------------------------------------


} // namespace Implementation ------------------------------------------------------------------------------------------