#pragma once

#include "Octree/Implementation/Nodes/BaseNode.hpp"

namespace Octree {
namespace Implementation {
namespace Nodes {

template<class T, class ID>
class BaseBranch: public BaseNode<T, ID>
{
    protected:

        using base_type = BaseNode<T, ID>;
        //--------------------------------------------------------------------------------------------------------------

        template<class Tree>
        friend class References::SubTree;
        //--------------------------------------------------------------------------------------------------------------

        template<class Tree>
        friend class Iterator::BaseStream;
        //--------------------------------------------------------------------------------------------------------------

        template<class Tree>
        friend class Iterator::LeafStream;
        //--------------------------------------------------------------------------------------------------------------

    public:

        using typename base_type::size_type;
        using typename base_type::difference_type;
        using typename base_type::value_type;
        //--------------------------------------------------------------------------------------------------------------
 
        BaseBranch( size_type parendId  = ID::IndexMask, 
                    size_type childId   = ID::IndexMask,
                    size_type childSize = 0) noexcept;
        //==============================================================================================================

        auto& size(      size_type i) const noexcept                             { return *(mIds + 8 + i);             }
        auto  childIndex(size_type i) const noexcept                             { return childId(i) & ID::IndexMask;  }
        //--------------------------------------------------------------------------------------------------------------

        static constexpr auto NodeTyp()  noexcept                             { return NodeType<ID>::Type::BranchNode; }
        //--------------------------------------------------------------------------------------------------------------

    private:

        auto  childId(size_type i) const noexcept                                                { return *(mIds + i); }
        auto& childId(size_type i)       noexcept                                                { return *(mIds + i); }
        //--------------------------------------------------------------------------------------------------------------

        auto& setSize(size_type i) noexcept                                                  { return *(mIds + 8 + i); }
        //--------------------------------------------------------------------------------------------------------------

        size_type  mIds[16]{};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

template<class T, class ID>
BaseBranch<T, ID>::BaseBranch
(  
    size_type _parendId, 
    size_type _childId,
    size_type _childSize
) noexcept
{
    this->parendId() = _parendId;

    for (int i = 0; i < 8; ++i) 
    {
        childId(i) = _childId;
    }

    for (int i = 0; i < 8; ++i) 
    {
        setSize(i) = _childSize;
    }
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace Nodes 
} // namespace Implementation 
} // namespace Octree ==================================================================================================