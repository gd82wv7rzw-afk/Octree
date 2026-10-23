#pragma once

#include "Octree/Implementation/Implementation.h"
#include "Octree/Implementation/Nodes/Nodes.h"
#include "Octree/Implementation/TreeID.hpp"
#include "Octree/Implementation/Nodes/BaseBranch.hpp"
#include "Octree/Implementation/Nodes/BaseTree.hpp"
#include "Octree/Implementation/Nodes/BaseNode.hpp"
#include "Octree/Implementation/MetalBuffer.hpp"

namespace Octree::Implementation {

/// @brief Base class for an octree implementation.
///
/// Defines the node, branch, buffer, and tree types used by the octree.
///
/// @tparam T Element type stored in the leaf nodes.
/// @tparam TT Element type stored in the branch nodes.
template<
    class T,
    class TT
>
class MetalTree
{
public:

    /// @brief Type used to represent tree identifiers.
    using id_type = TreeID<int>;

    /// @brief Type representing a branch node.
    using branch_type = Nodes::BaseBranch<TT, id_type>;

    /// @brief Buffer type used to store branch nodes.
    using branchBuffer_type = MetalBuffer<branch_type>;

    /// @brief Type representing a leaf node.
    using leaf_type = Nodes::BaseNode<T, id_type>;

    /// @brief Buffer type used to store leaf nodes.
    using leafBuffer_type = MetalBuffer<leaf_type>;

    /// @brief Type representing the underlying tree structure.
    using tree_type = Nodes::BaseTree<leafBuffer_type, branchBuffer_type>;

    /// @brief Type used for sizes and indices.
    using size_type = id_type::size_type;

    /// @brief Type used to represent differences between indices.
    using difference_type = id_type::difference_type;
    //------------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Octree::Implementation
