#pragma once

#include "Octree/Implementation/Traits/TreeTraits.hpp"
#include "Octree/Implementation/References/BaseReference.hpp"

namespace Octree::Implementation::Iterator {

/**
 * @brief Provides the common functionality for tree iterators.
 *
 * BaseStream stores a reference to a tree and the index of the
 * current branch. It provides navigation through the tree hierarchy.
 *
 * @tparam Tree The tree type traversed by this stream.
 */
template<class Tree>
class BaseStream
    : public References::BaseReference<
          typename Traits::TreeTraits<Tree>::branch_type>
{
    friend Tree;

protected:
    using tree_type = Traits::TreeTraits<Tree>;
    using base_type = References::BaseReference<typename tree_type::branch_type>;
    using id_type = Tree::id_type;

    using base_type::data;
    using base_type::isConst;

    using reference = Tree::reference;
    using const_reference = Tree::const_reference;

public:
    using size_type = typename id_type::size_type;
    using difference_type = typename id_type::difference_type;

    /**
     * @brief Constructs a stream for the specified tree and branch.
     *
     * @param tree Pointer to the tree.
     * @param index Index of the initial branch.
     */
    constexpr BaseStream(Tree* tree = nullptr, size_type index = 0) noexcept;

    /**
     * @brief Returns a reference to the current branch.
     *
     * @return A const reference to the current branch.
     */
    auto operator*() const noexcept
    {
        return const_reference(mTree, mIndex);
    }

    /**
     * @brief Returns a mutable reference to the current branch.
     *
     * @return A reference to the current branch.
     */
    auto operator*() noexcept requires (!isConst)
    {
        return reference(mTree, mIndex);
    }

    /**
     * @brief Provides pointer-like access to the current branch.
     *
     * @return Pointer to the current branch data.
     */
    auto* operator->() const noexcept
    {
        return data();
    }

    /**
     * @brief Provides mutable pointer-like access to the current branch.
     *
     * @return Pointer to the current branch data.
     */
    auto* operator->() noexcept requires (!isConst)
    {
        return data();
    }

    /**
     * @brief Returns the index of the current branch.
     */
    size_type index() const
    {
        return mIndex;
    }

    /**
     * @brief Advances the stream to the next position.
     */
    BaseStream& operator++() noexcept;

    /**
     * @brief Advances the stream and returns its previous state.
     */
    BaseStream operator++(int) noexcept
    {
        auto t = *this;
        ++(*this);
        return t;
    }

    /**
     * @brief Moves the stream to the previous position.
     */
    BaseStream& operator--() noexcept;

    /**
     * @brief Moves the stream backward and returns its previous state.
     */
    BaseStream operator--(int) noexcept
    {
        auto t = *this;
        --(*this);
        return t;
    }

    /**
     * @brief Moves the stream to the parent branch.
     */
    BaseStream& goUp() noexcept;

    /**
     * @brief Moves the stream to the deepest descendant branch.
     */
    BaseStream& goToDeapestPossition() noexcept;

protected:
    /**
     * @brief Returns a pointer to the branch at the specified index.
     *
     * @param i Index of the branch.
     * @return Pointer to the requested branch.
     */
    constexpr auto* branch(size_type i) const noexcept
    {
        return mTree->branches()->getPointer(i);
    }

    /**
     * @brief Returns a mutable pointer to the branch at the specified index.
     *
     * @param i Index of the branch.
     * @return Pointer to the requested branch.
     */
    constexpr auto* branch(size_type i) noexcept
    {
        return mTree->branches()->getPointer(i);
    }

    Tree* mTree{nullptr};
    size_type mIndex{0};
};

//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
constexpr BaseStream<Tree>::BaseStream(Tree* tree, size_type index) noexcept
    : base_type(),
      mIndex(index),
      mTree(tree)
{
    if (tree != nullptr)
        base_type::operator=(branch(index));
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief Advances the stream to the next branch in traversal order.
 */
template<class Tree>
BaseStream<Tree>&
BaseStream<Tree>::operator++() noexcept
{
    if (!bool(*this))
        return *this;

    mIndex = data()->parentIndex();
    char oct = data()->possitonInParent() + 1;
    data() = branch(mIndex);

    if (!bool(*this))
        return *this;

    for (; oct < 8; ++oct)
    {
        if (data()->childId(oct) & id_type::BranchBit)
        {
            mIndex = data()->childIndex(oct);
            base_type::operator=(branch(mIndex));
            return goToDeapestPossition();
        }
    }

    return *this;
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief Moves the stream to the previous branch in traversal order.
 */
template<class Tree>
BaseStream<Tree>&
BaseStream<Tree>::operator--() noexcept
{
    char oct = 7;

    while (bool(*this))
    {
        for (; !(oct & 8); --oct)
        {
            if (data()->childId(oct) & id_type::BranchBit)
            {
                mIndex = data()->childId(oct) & id_type::IndexMask;
                base_type::operator=(branch(mIndex));
                return *this;
            }
        }

        auto parendId = data()->parendId();
        mIndex = parendId & id_type::IndexMask;
        oct = id_type::OctantToNumber(parendId) - 1;
        base_type::operator=(branch(mIndex));
    }

    return *this;
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief Moves the stream to the parent branch.
 */
template<class Tree>
BaseStream<Tree>&
BaseStream<Tree>::goUp() noexcept
{
    if (!bool(*this))
        return *this;

    mIndex = data()->parendId() & id_type::IndexMask;
    base_type::operator=(branch(mIndex));

    return *this;
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * @brief Moves the stream to the deepest descendant branch.
 */
template<class Tree>
BaseStream<Tree>&
BaseStream<Tree>::goToDeapestPossition() noexcept
{
    if (!bool(*this))
        return *this;

    auto oct = 0;

    for (; oct < 8; ++oct)
    {
        if (data()->childId(oct) & id_type::BranchBit)
        {
            mIndex = data()->childId(oct) & id_type::IndexMask;

            base_type::operator=(branch(mIndex));
            oct = 0;
        }
    }

    return *this;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace Octree::Implementation::Iterator