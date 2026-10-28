
#pragma once

#include <memory>
#include <cstddef>

#include <Metal/Metal.hpp>


struct BufferDeleter
{
    inline void operator()(MTL::Buffer* buffer) const noexcept
    {
        if (buffer) buffer->release();
    }
};
//----------------------------------------------------------------------------------------------------------------------

template<typename T> class MetalArray
{
public:

    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;

    using iterator        = T*;
    using const_iterator  = const T*;

    using BufferPtr = std::unique_ptr<MTL::Buffer, BufferDeleter>;

public:

    explicit MetalArray(MTL::Device* device, size_t count)
        : mSize(count)
        , mBuffer(device->newBuffer(sizeof(T) * count,MTL::ResourceStorageModeShared))
    {
        if (!mBuffer) throw std::runtime_error("Failed to create Metal buffer");
        memset(mBuffer->contents(), 0, mSize * sizeof(T));
    }
    //------------------------------------------------------------------------------------------------------------------

    ~MetalArray() = default;
    //------------------------------------------------------------------------------------------------------------------

    MetalArray(const MetalArray&) = delete;
    //------------------------------------------------------------------------------------------------------------------

    MetalArray& operator=(const MetalArray&) = delete;
    //------------------------------------------------------------------------------------------------------------------

    MetalArray(MetalArray&&) noexcept = default;
    //------------------------------------------------------------------------------------------------------------------

    MetalArray& operator=(MetalArray&&) noexcept = default;
    //------------------------------------------------------------------------------------------------------------------

public:

    [[nodiscard]] inline pointer data() noexcept { return reinterpret_cast<pointer>( mBuffer->contents()); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_pointer data() const noexcept { return reinterpret_cast<const_pointer>( mBuffer->contents()); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline size_t size() const noexcept { return mSize; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline size_t bytes() const noexcept { return mSize * sizeof(T); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline bool empty() const noexcept { return mSize == 0; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline reference operator[](size_t index) noexcept { return data()[index]; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_reference operator[](size_t index) const noexcept { return data()[index]; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline reference front() noexcept { return data()[0]; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_reference front() const noexcept { return data()[0]; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline reference back() noexcept { return data()[mSize - 1]; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_reference back() const noexcept { return data()[mSize - 1]; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline iterator begin() noexcept { return data(); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_iterator begin() const noexcept { return data(); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_iterator cbegin() const noexcept { return data(); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline iterator end() noexcept { return data() + mSize; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_iterator end() const noexcept { return data() + mSize; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const_iterator cend() const noexcept { return data() + mSize; }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline MTL::Buffer* buffer() noexcept { return mBuffer.get(); }
    //------------------------------------------------------------------------------------------------------------------

    [[nodiscard]] inline const MTL::Buffer* buffer() const noexcept { return mBuffer.get(); }
    //------------------------------------------------------------------------------------------------------------------

private:

    size_t    mSize {0};
    BufferPtr mBuffer;
};
//----------------------------------------------------------------------------------------------------------------------
