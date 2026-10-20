#pragma once

#include "Octree/Implementation/Iterator/BaseStream.hpp"
#include "Octree/Implementation/References/SubTree.hpp"
#include "Octree/Implementation/TreeID.hpp"

namespace Octree {
namespace Implementation {
namespace Nodes {

template<class LeafBuf, class BranchBuf>
class BaseTree
{
    private:

        template<class> 
        friend class References::SubTree;

        template<class>
        friend class Iterator::LeafStream;

        template<class>
        friend class Iterator::BaseStream;

        template<class Tree>
        friend class Traits::TreeTraits;
        //--------------------------------------------------------------------------------------------------------------

        using id_type           = TreeID<typename BranchBuf::size_type>;
        using branch_type       = BranchBuf::value_type;
        using branchBuffer_type = BranchBuf;
        using leaf_type         = LeafBuf::value_type;
        using leafBuffer_type   = LeafBuf;
    
    public:    
        using size_type         = BranchBuf::size_type;
        using difference_type   = BranchBuf::difference_type;
        using const_iterator    = Iterator::BaseStream<const BaseTree>;
        using       iterator    = Iterator::BaseStream<      BaseTree>;

        using       reference   = References::SubTree<      BaseTree>;
        using const_reference   = References::SubTree<const BaseTree>;
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto begin()       noexcept               { return         iterator(this, 0).goToDeapestPossition(); }
        constexpr auto end()         noexcept               { return       ++iterator(this, 0);                        }
        constexpr auto begin() const noexcept               { return   const_iterator(this, 0).goToDeapestPossition(); }
        constexpr auto end()   const noexcept               { return ++const_iterator(this, 0);                        }
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto root()       noexcept                                        { return       reference(this, 0); }
        constexpr auto root() const noexcept                                        { return const_reference(this, 0); }
        //--------------------------------------------------------------------------------------------------------------

    //private:

        constexpr auto* branches()       noexcept                                                 { return &mBranches; }
        constexpr auto* branches() const noexcept                                                 { return &mBranches; }
        constexpr auto* leafs()          noexcept                                                 { return &mLeafs;    }
        constexpr auto* leafs()    const noexcept                                                 { return &mLeafs;    }
        //--------------------------------------------------------------------------------------------------------------

    private:

        constexpr iterator deleatSubTree(size_type idx) noexcept;
        void updateSize(iterator it, difference_type inc) noexcept; 
        //--------------------------------------------------------------------------------------------------------------

        BranchBuf mBranches{};
        LeafBuf   mLeafs{};
        size_type mNumerOfBranches{0};
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Nodes 
} // namespace Implementation 
} // namespace Octree ==================================================================================================
