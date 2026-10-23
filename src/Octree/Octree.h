#pragma once

#include "Octree/Implementation/TreeBase.hpp"
#include "Octree/Implementation/MetalTree.hpp"

#include <vector>
#include <memory>

namespace Octree {
    
    /// @brief A generic Octree data structure.
    /// 
    /// @tparam T The data type stored in the tree (e.g., Particle, Body).
    /// @tparam TT The internal data type used for calculations (defaults to T).
    /// @tparam ST The size/index type used for addressing (defaults to int).
    /// @tparam Container The underlying container type (defaults to std::vector).
    /// @tparam Allocator The allocator used by the container (defaults to std::allocator).
    template
    <
        class T, 
        class TT = T,
        typename ST = int,
        template<class, class>  class Container = std::vector,
        template<class>         class Allocator = std::allocator
    > 
    using Tree = typename Octree::Implementation::TreeBase<T, TT, ST, Container, Allocator>::tree_type;
    //------------------------------------------------------------------------------------------------------------------

    /// @brief A generic Octree data structure on the GPU.
    template<class T, class TT = T>
    using MetalTree = Implementation::MetalTree<T, TT>;

} // namespace Octree