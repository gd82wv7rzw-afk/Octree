#pragma once

#include "Octree/Implementation/Nodes/Nodes.h"
#include "Octree/Implementation/Nodes/BaseNode.hpp"

namespace Octree {
namespace Implementation {
namespace Nodes {

template<class ID>
class SubTree: public BaseNode<typename ID::size_type[16], ID>
{
    public:
       
        using base_type = BaseNode<typename ID::size_type[16], ID>;
        using typename base_type::size_type;
        using typename base_type::difference_type;
        using typename base_type::value_type;
        using reference = ID::reference;
        using const_reference = ID::const_reference;
        using pointer = ID::pointer;
        using const_pointer = ID::const_pointer;      
        //--------------------------------------------------------------------------------------------------------------    

        template<class NodeT>
        static constexpr SubTreeInterface<NodeT> 
        Interface(NodeT* node = nullptr) noexcept                           { return SubTreeInterface<NodeT>(node); }
        //--------------------------------------------------------------------------------------------------------------

        template<class NodeT>
        static constexpr SubTreeInterface<const NodeT> 
        Interface(const NodeT* node = nullptr) noexcept               { return SubTreeInterface<const NodeT>(node); }
        //--------------------------------------------------------------------------------------------------------------

        auto interface()       noexcept                                    { return Interface<      SubTree>(this); }
        auto interface() const noexcept                                    { return Interface<const SubTree>(this); }
        //--------------------------------------------------------------------------------------------------------------

        auto operator*() noexcept                                                { return interface();                 }
        auto operator*() const noexcept                                          { return interface();                 }
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto* begin()       noexcept                                           { return *base_type::begin(); }
        constexpr auto* end()         noexcept                                           { return *base_type::end();   }
        constexpr auto* begin() const noexcept                                           { return *base_type::begin(); }
        constexpr auto* end()   const noexcept                                           { return *base_type::end();   }
        //--------------------------------------------------------------------------------------------------------------
        
        constexpr auto* childId()                                  { return begin(); }
        constexpr auto* childId()   const                          { return begin(); }
        constexpr auto* childSize()                                { return begin() + 8; }
        constexpr auto* childSize() const                          { return begin() + 8; }
        //--------------------------------------------------------------------------------------------------------------
        
        constexpr SubTree() noexcept;
        //==============================================================================================================

    private:

        //template<class> friend class BaseBranchInterface;
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------


template<class SubTree>
class SubTreeInterface: public NodeInterface<SubTree>
{
    protected:

        using NodeInterface<SubTree>::isConst; 
        using typename NodeInterface<SubTree>::nodePtr_type;
        //--------------------------------------------------------------------------------------------------------------

    public:

        using base_type = NodeInterface<SubTree>;
        using typename base_type::id_type;
        using typename base_type::size_type;
        using typename base_type::difference_type;
        using octant_type = id_type::octant_type;
        //--------------------------------------------------------------------------------------------------------------

        explicit SubTreeInterface(nodePtr_type node) noexcept: base_type(node){}
        SubTreeInterface() noexcept = default;
        //============================================================================================================== 
        
        template<class OT> 
        requires(isConst && !std::is_const_v<OT>) SubTreeInterface(const SubTreeInterface<OT>& o) noexcept;
        //==============================================================================================================

        constexpr bool isBranch(auto i   ) const { return (base_type::BranchBit & *(this->root().childId() + i) ); }
        constexpr bool isLeaf(auto i     ) const { return (base_type::LeafBit & *(this->root().childId() + i));    }
        constexpr bool isDeleated(auto i ) const { return (base_type::DeletedBit & *(this->root().childId() + i));}
        constexpr bool isEmpty(auto i    ) const { return (*(this->root().childId() + i) & id_type::IndexMask) == (base_type::InvalidIndex); }
        constexpr bool isEmpty()           const;
        //constexpr bool parentExist()       const { return this->root()->parent()->data() & (this->BranchBit);}
        //--------------------------------------------------------------------------------------------------------------

        constexpr void  setTypeOf( auto i, NodeType typ ) requires (!isConst );
        constexpr auto  typeOf(    auto i               ) const;
};
//----------------------------------------------------------------------------------------------------------------------

template<class ID>
constexpr SubTree<ID>::SubTree() noexcept
{
    for (auto i = 0; i < 8; ++i) 
    {
        childId()[i] = interface().InvalidIndex;
        childSize()[i] = 0;
    }

    this->parentId().setIndex(interface().InvalidIndex);
}
//----------------------------------------------------------------------------------------------------------------------

template<class SubTree>
constexpr void 
SubTreeInterface<SubTree>::setTypeOf( auto i, NodeType typ ) requires (!isConst )
{ 
    size_type& data = *(this->root()->childId() + i);

    // clear the octant
    data &= id_type::IndexMask;

    switch (typ) 
    {
        case NodeType::BranchNode: { data |= base_type::BranchBit;      break; } 
        case NodeType::LeafNode:   { data |= base_type::LeafBit;        break; } 
        case NodeType::Deleated:   { data |= base_type::DeletedBit;    break; } 
        default:                   {                                    break; }                      
    }
}
//----------------------------------------------------------------------------------------------------------------------

template<class SubTree>
constexpr auto
SubTreeInterface<SubTree>::typeOf( auto i) const
{ 
    size_type data = *(this->root()->childId() + i) & id_type::OctantMask;
    {
        if (data & base_type::BranchBit)   return NodeType::BranchNode; 
        if (data & base_type::LeafBit)     return NodeType::LeafNode; 
        if (data & base_type::DeletedBit) return NodeType::Deleated;  
        /*default:*/                       return NodeType::EmptyNode;                   
    }
}
//----------------------------------------------------------------------------------------------------------------------

template<class SubTree>
constexpr bool SubTreeInterface<SubTree>::isEmpty() const
{
    auto mask = base_type::BranchBit | base_type::LeafBit;

    for (auto i = 0; i < 8; ++i) 
    {
        if( this->root()->childId()[i] & mask) return false; 
    }

    return true;
}
//----------------------------------------------------------------------------------------------------------------------

//template<class BaseBranch>
//BaseBranchInterface(BaseBranch* node) -> BaseBranchInterface<BaseBranch>;

template<class SubTree>
SubTreeInterface(const SubTree* node) -> SubTreeInterface<const SubTree>;
//----------------------------------------------------------------------------------------------------------------------

} // namespace Nodes 
} // namespace Implementation 
} // namespace Octree ==================================================================================================