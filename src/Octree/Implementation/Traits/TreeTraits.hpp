#pragma once

#include <type_traits>

namespace Octree::Implementation::Traits {

template<class Tree>
struct TreeTraits
{
    static const bool Const = std::is_const_v<Tree>;
    //------------------------------------------------------------------------------------------------------------------

    using branch_type = std::conditional_t<       
        Const, 
            const typename Tree::branch_type,       
            typename Tree::branch_type>;
    //------------------------------------------------------------------------------------------------------------------

    using branchBuffer_type = std::conditional_t< 
        Const, 
            const typename Tree::branchBuffer_type, 
            typename Tree::branchBuffer_type>;
    //------------------------------------------------------------------------------------------------------------------
    
    using leaf_type = std::conditional_t<       
        Const, 
            const typename Tree::leaf_type,       
            typename Tree::leaf_type>;
    //------------------------------------------------------------------------------------------------------------------

    using leafBuffer_type = std::conditional_t< 
        Const, 
            const typename Tree::leafBuffer_type, 
            typename Tree::leafBuffer_type>;
    //------------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace =========================================================================================================