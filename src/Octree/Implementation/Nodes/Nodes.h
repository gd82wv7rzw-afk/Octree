#pragma once


namespace Octree {
namespace Implementation {
namespace Nodes {

    template<class ID>
    struct NodeType
    {
        enum class Type: typename ID::size_type
        {
            LeafNode   = ID::LeafBit,
            BranchNode = ID::BranchBit,
            Deleated   = ID::DeletedBit,
            EmptyNode  = ID::IndexMask
        };
    };
    //------------------------------------------------------------------------------------------------------------------

    template<class T, class ID >
    class BaseNode;
    //------------------------------------------------------------------------------------------------------------------

    template<class T, class ID>
    class BaseBranch;
    //------------------------------------------------------------------------------------------------------------------
    
    template<class LeafBuf, class BranchBuf>
    class BaseTree;
    //------------------------------------------------------------------------------------------------------------------

} // namespace Nodes 
} // namespace Implementation 
} // namespace Octree ==================================================================================================
