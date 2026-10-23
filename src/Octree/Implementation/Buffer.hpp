#pragma once

#include "Octree/Implementation/TreeID.hpp"
#include "Octree/Implementation/Nodes/Nodes.h"

#include <iterator>

namespace Octree {
namespace Implementation {

/// @brief A buffer that stores nodes and reuses deleted entries.
///
/// The buffer maintains a free list of deleted entries using the parent ID
/// field of each node. This allows deleted entries to be reused without
/// allocating additional storage for free-list management.
///
/// @tparam C The underlying container type.
template<class C>
class Buffer
{
private:

    using container_type = C;
    using id_type        = TreeID<typename C::value_type::size_type>;
    using MyNodeType     = Nodes::NodeType<id_type>::Type;

public:

    /// @brief The type of elements stored in the buffer.
    using value_type = container_type::value_type;

    /// @brief The type used for indices and sizes.
    using size_type = id_type::size_type;

    /// @brief The type used for index differences.
    using difference_type = id_type::difference_type;

    /// @brief Reference to a stored element.
    using reference = container_type::reference;

    /// @brief Const reference to a stored element.
    using const_reference = container_type::const_reference;

    /// @brief Pointer to a stored element.
    using pointer = container_type::pointer;

    /// @brief Const pointer to a stored element.
    using const_pointer = container_type::const_pointer;
    //--------------------------------------------------------------------------------------------------

    /// @brief Returns a pointer to the element at the specified index.
    ///
    /// @param i Index of the element.
    /// @return Pointer to the element, or nullptr if the index is invalid
    ///         or the element has been deleted.
    constexpr pointer getPointer(size_type i) noexcept;
    //--------------------------------------------------------------------------------------------------

    /// @brief Returns a const pointer to the element at the specified index.
    ///
    /// @param i Index of the element.
    /// @return Const pointer to the element, or nullptr if the index is
    ///         invalid or the element has been deleted.
    constexpr const_pointer getPointer(size_type i) const noexcept;
    //--------------------------------------------------------------------------------------------------

    /// @brief Provides indexed access to an element.
    ///
    /// @param i Index of the element.
    /// @return Reference to the element.
    constexpr reference operator[](size_type i)
    {
        return *getPointer(i);
    }
    //--------------------------------------------------------------------------------------------------

    /// @brief Provides const indexed access to an element.
    ///
    /// @param i Index of the element.
    /// @return Const reference to the element.
    constexpr const_reference operator[](size_type i) const
    {
        return *getPointer(i);
    }
    //--------------------------------------------------------------------------------------------------

    /// @brief Returns the next available insertion index.
    ///
    /// If the free list is empty, the container size is returned.
    /// Otherwise, the index of the first reusable entry is returned.
    ///
    /// @return Index of the next insertion position.
    constexpr size_type nextIndex() const noexcept
    {
        return mNextIndex == id_type::IndexMask
            ? mContainer.size()
            : mNextIndex;
    }
    //--------------------------------------------------------------------------------------------------

    /// @brief Inserts an element into the buffer.
    /// Appends the element if no deleted entries are available; otherwise,
    /// reuses the first entry in the free list.
    /// @param x Element to insert.
    void push_back(const_reference x) noexcept;
    //--------------------------------------------------------------------------------------------------

    /// @brief Marks an element as deleted and adds it to the free list.
    /// @param i Index of the element to delete.
    constexpr void deleate(size_type i) noexcept;

private:

    /// @brief Container holding the elements.
    container_type mContainer{};

    /// @brief Index of the first deleted entry, or IndexMask if none exist.
    size_type mNextIndex{id_type::IndexMask};
    //--------------------------------------------------------------------------------------------------
};

//==================================================================================================
// Implementation
//==================================================================================================

/// @brief Returns a pointer to a valid, non-deleted element.
template<class C>
constexpr typename Buffer<C>::pointer
Buffer<C>::getPointer(size_type i) noexcept
{
    if (!(i < mContainer.size())) {
        return nullptr;
    }

    auto it = mContainer.begin();
    std::advance(it, i);

    return &*it;
}

//--------------------------------------------------------------------------------------------------

/// @brief Returns a const pointer to a valid, non-deleted element.
template<class C>
constexpr typename Buffer<C>::const_pointer
Buffer<C>::getPointer(size_type i) const noexcept
{
    if (!(i < mContainer.size())) {
        return nullptr;
    }

    auto it = mContainer.begin();
    auto idx = it->parendId();

    std::advance(it, i);

    return &*it;
}

//--------------------------------------------------------------------------------------------------

/// @brief Inserts an element, reusing a deleted entry if possible.
template<class C>
void
Buffer<C>::push_back(const_reference x) noexcept
{
    if (mNextIndex == id_type::IndexMask) {
        mContainer.push_back(x);
    }
    else {
        auto* p = getPointer(mNextIndex);

        auto idx = p->parendId() & id_type::IndexMask;
        mNextIndex = idx;
        *p = x;
    }
}
//--------------------------------------------------------------------------------------------------

/// @brief Marks an element as deleted and adds it to the free list.
template<class C>
constexpr void
Buffer<C>::deleate(size_type i) noexcept
{
    auto* p = getPointer(i);

    if (p == nullptr) {
        return;
    }

    auto idx = mNextIndex;
    mNextIndex = i;

    p->parendId() = id_type::DeletedBit | idx;
}
//--------------------------------------------------------------------------------------------------

} // namespace Implementation
} // namespace Octree
