#pragma once

#include "Octree/Implementation/Iterator/LeafStream.hpp"
#include "Octree/Implementation/Nodes/Nodes.h"
#include "Octree/Implementation/References/BaseReference.hpp"
#include "Octree/Implementation/Traits/TreeTraits.hpp"
#include <cstddef>
#include <numeric>

namespace Octree::Implementation::References {

template<class Tree>
class SubTree: public BaseReference< typename Traits::TreeTraits<Tree>::branch_type >
{
    private:
        
        friend Tree;
        friend Tree::iterator;
        friend Tree::const_iterator;
        //--------------------------------------------------------------------------------------------------------------

    protected:

        using base_type = BaseReference< typename Traits::TreeTraits<Tree>::branch_type >;
        using id_type   = Tree::id_type;
        using ID = id_type::Id;
        using branch_type = Traits::TreeTraits<Tree>::branch_type;
        using typename base_type::Pointer;
        using base_type::data;
        using base_type::isConst;
        using TreePtr = std::conditional_t<isConst, const Tree*, Tree*>;
        using leaf_type = Tree::leaf_type;
        static const typename id_type::size_type OccupiedMask = id_type::BranchBit | id_type::LeafBit;
        //--------------------------------------------------------------------------------------------------------------

    public:

        using iterator = Octree::Implementation::Iterator::LeafStream<Tree>;
        using const_iterator = Octree::Implementation::Iterator::LeafStream<const Tree>;
        using size_type = id_type::size_type;
        using difference_type = id_type::difference_type;
        //--------------------------------------------------------------------------------------------------------------

        constexpr SubTree(TreePtr tree, size_type index = 0) noexcept;
        //==============================================================================================================

        constexpr auto size()            const noexcept {return std::accumulate(&data()->size(0), &data()->size(8), 0);}
        constexpr auto size( size_type i)const noexcept {return data()->size(i);                                       }
        constexpr auto index(size_type i)const noexcept {return data()->childIndex(i);                                 }
        constexpr auto index()           const noexcept {return mIndex;                                                }
        //--------------------------------------------------------------------------------------------------------------
        
        bool isOccupied(size_type i) const noexcept               { return (data()->childId(i) &  OccupiedMask);       }
        bool isLeaf(    size_type i) const noexcept               { return (data()->childId(i) &  id_type::LeafBit);   }
        bool isBranch(  size_type i) const noexcept               { return (data()->childId(i) &  id_type::BranchBit); }
        bool isEmpty(   size_type i) const noexcept               { return (data()->childId(i) == id_type::IndexMask); }
        auto typeOf(    size_type i) const noexcept;
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto& branch( size_type i) const                         { return *branchPtr(data()->childIndex(i)); }
        constexpr auto& branch( size_type i)                               { return *branchPtr(data()->childIndex(i)); }
        constexpr auto& leaf(   size_type i) const                         { return *leafPtr(  data()->childIndex(i)); }
        constexpr auto& leaf(   size_type i)                               { return *leafPtr(  data()->childIndex(i)); }
        constexpr auto& parend()             const noexcept                { return *branchPtr(data()->parentIndex()); }
        constexpr auto& parend()                   noexcept                { return *branchPtr(data()->parentIndex()); }
        constexpr auto& self()               const noexcept                { return *data();                           }
        constexpr auto& self()                     noexcept                { return *data();                           }
        //--------------------------------------------------------------------------------------------------------------

        auto     newSubTree(size_type i) noexcept requires(!isConst)                 { return (this->*mNewSubTree)(i); }
        auto     newLeaf(   size_type i) noexcept requires(!isConst)                 { return (this->*mNewLeaf)(i);    }
        SubTree& deleate(   size_type i) noexcept requires(!isConst);
        //--------------------------------------------------------------------------------------------------------------

        SubTree& goIn(size_type i)    noexcept; 
        SubTree& goTo(size_type idx)  noexcept;
        SubTree& goUp()               noexcept;
        //--------------------------------------------------------------------------------------------------------------

        auto begin()                                                { return         iterator(mTree, findFirstLeaf()); }
        auto begin() const                                          { return   const_iterator(mTree, findFirstLeaf()); }
        auto end()                                                  { return       ++iterator(mTree, findLastLeaf());  }
        auto end()   const                                          { return ++const_iterator(mTree, findLastLeaf());  }
        //--------------------------------------------------------------------------------------------------------------

        auto subTreesBegin()           { return   typename Tree::      iterator(mTree, mIndex).goToDeapestPossition(); }
        auto subTreesBegin() const     { return   typename Tree::const_iterator(mTree, mIndex).goToDeapestPossition(); }
        auto subTreesEnd()             { return ++typename Tree::      iterator(mTree, mIndex);                        }
        auto subTreesEnd()   const     { return ++typename Tree::const_iterator(mTree, mIndex);                        }
        //--------------------------------------------------------------------------------------------------------------

    private:

        using newSub   = SubTree  (SubTree::*)(size_type i) noexcept;
        using newLeafP = iterator (SubTree::*)(size_type i) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        auto updateSize(difference_type inc) noexcept requires(!isConst);
        //--------------------------------------------------------------------------------------------------------------
      
        auto findFirstLeaf() const noexcept; 
        auto findLastLeaf()  const noexcept; 
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto* buffer()                const noexcept                   { return mTree->branches();           }
        constexpr auto* buffer()                      noexcept                   { return mTree->branches();           }
        constexpr auto* leafBuffer()            const noexcept                   { return mTree->leafs();              }
        constexpr auto* leafBuffer()                  noexcept                   { return mTree->leafs();              }
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto* branchPtr( size_type i) const noexcept                   { return buffer()->getPointer(i);     }
        constexpr auto* branchPtr( size_type i)       noexcept                   { return buffer()->getPointer(i);     }
        constexpr auto* leafPtr(   size_type i) const noexcept                   { return leafBuffer()->getPointer(i); }
        constexpr auto* leafPtr(   size_type i)       noexcept                   { return leafBuffer()->getPointer(i); }
        //--------------------------------------------------------------------------------------------------------------

        SubTree  newSubTree_init(size_type i) noexcept requires(!isConst);
        SubTree  newSubTree_run( size_type i) noexcept requires(!isConst);
        iterator newLeaf_init(   size_type i) noexcept requires(!isConst);
        iterator newLeaf_run(    size_type i) noexcept requires(!isConst);
        //--------------------------------------------------------------------------------------------------------------

        size_type mIndex{      0       };
        TreePtr   mTree{       nullptr };
        mutable newSub    mNewSubTree{ nullptr };
        mutable newLeafP  mNewLeaf{    nullptr };
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
constexpr SubTree<Tree>::SubTree(TreePtr tree, size_type index) noexcept: 
    base_type(), 
    mIndex(index), 
    mTree(tree)              
{ 
    assert ( mTree != nullptr );   
    base_type::data() = (branchPtr(index)); 

    if constexpr (!isConst) 
    {
        if (mTree->mNumberOfBranches > 0) 
        {
            mNewSubTree = &SubTree::newSubTree_run;
            mNewLeaf    = &SubTree::newLeaf_run;
        }
        else 
        {
            mNewSubTree = &SubTree::newSubTree_init;
            mNewLeaf    = &SubTree::newLeaf_init;
        }   
    }
}
//======================================================================================================================

template<class Tree>
auto
SubTree<Tree>::typeOf(size_type i) const  noexcept
{
    size_type id = data()->childId(i);
    
    if(id & id_type::LeafBit)       return Nodes::NodeType<id_type>::Type::LeafNode;
    if(id & id_type::BranchBit)     return Nodes::NodeType<id_type>::Type::BranchNode;
    if(id & id_type::DeletedBit)   return Nodes::NodeType<id_type>::Type::Deleated;
                                    return Nodes::NodeType<id_type>::Type::EmptyNode;   

}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>
SubTree<Tree>::newSubTree_init(size_type i) noexcept requires(!isConst)
{
    assert ( mTree != nullptr ); 

    // the first of all branch / subTree
    branch_type newBr; 
    newBr.parendId() = id_type::IndexMask;

    mIndex = buffer()->nextIndex();
    buffer()->push_back(newBr);

    // update the internal pointer
    base_type::operator = (branchPtr(mIndex));

    ++mTree->mNumberOfBranches;
    mTree->mRootID = mIndex;

    // now we are a valid sub tree :-)
    mNewSubTree = &SubTree::newSubTree_run;
    mNewLeaf    = &SubTree::newLeaf_run;

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree> 
SubTree<Tree>::newSubTree_run(size_type i) noexcept requires(!isConst)
{
    assert ( mTree != nullptr ); 

    auto& childIdl = data()->childId(i);

    if ( OccupiedMask & childIdl) return SubTree(nullptr);

    branch_type newBr; 

    auto idx = buffer()->nextIndex();

    // set the type bit
    childIdl  = idx | id_type::BranchBit;

    // in the upper 3 bits the currend counter is stored and the index in the lower bits
    newBr.parendId() = id_type::NumberToOctant(i) | mIndex;

    buffer()->push_back(newBr);

    // update the internal pointer
    base_type::operator = (branchPtr(mIndex));

    ++mTree->mNumberOfBranches;

    return SubTree(mTree, idx);
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>::iterator
SubTree<Tree>::newLeaf_init(size_type i) noexcept requires(!isConst)
{
    assert ( mTree != nullptr );

    // we nead a valid sub tree to create a new leaf
    newSubTree(i);
    return newLeaf(i);
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>::iterator
SubTree<Tree>::newLeaf_run(size_type i) noexcept requires(!isConst)
{
    assert ( mTree != nullptr );

    auto& childIdl = data()->childId(i);

    // if is occupied return default
    if (OccupiedMask & childIdl) return iterator(mTree, id_type::IndexMask);

    auto idx = leafBuffer()->nextIndex();

    // set the type bit
    childIdl  = idx | id_type::LeafBit;

    leaf_type newLeaf;

    // in the upper 3 bits the currend counter is stored and the index of the root in the lower bits
    newLeaf.parendId() = id_type::NumberToOctant(i) | mIndex;

    leafBuffer()->push_back(newLeaf);

    ++data()->setSize(i);

    updateSize(1);

    auto it = iterator(mTree, idx);

    return it;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
auto 
SubTree<Tree>::updateSize(difference_type inc) noexcept requires(!isConst)
{
    auto idx  = data()->parentIndex();
    auto oct  = data()->possitonInParent();

    while (idx != id_type::IndexMask) 
    {
        auto root = branchPtr(idx);
        root->setSize(oct) += inc;

        oct = root->possitonInParent();
        idx = root->parentIndex();
    }
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>::SubTree& 
SubTree<Tree>::goIn(size_type i) noexcept 
{
    mIndex = data()->childId(i) & id_type::IndexMask; 
    data() = branchPtr(mIndex);
    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>::SubTree& 
SubTree<Tree>::goTo(size_type idx) noexcept
{
    mIndex = idx;
    data() = branchPtr(mIndex);
    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>::SubTree& 
SubTree<Tree>::goUp() noexcept
{
    mIndex = data()->parentIndex();
    data() = mIndex != id_type::IndexMask ? branchPtr(mIndex) : nullptr;
    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
SubTree<Tree>::SubTree& 
SubTree<Tree>::deleate(size_type i) noexcept requires(!isConst)
{
    if (isLeaf(i))
    {
        auto idx = index(i);
        leafBuffer()->deleate(idx); 

        data()->setSize(i) = 0;
        data()->childId(i) = id_type::IndexMask;

        updateSize(-1);
        return *this;
    }

    if (isBranch(i)) 
    {
        auto idx = index(i);
        auto sz  = data()->size(i);

        data()->setSize(i) = 0;
        data()->childId(i) = id_type::IndexMask;

        updateSize(-sz);

        auto it = typename Tree::iterator(mTree, idx).goToDeapestPossition();
        auto oct = 0;

        while (true) 
        {
            for (; oct < 8; ++oct) 
            {
                auto idx = (*it).index(oct);

                if ((*it).isLeaf(oct))
                {
                    leafBuffer()->deleate(idx); 
                }
            }

            idx = it.index();
            oct = it->possitonInParent() + 1;
            
            it.goUp();

            buffer()->deleate(idx);
            --mTree->mNumberOfBranches; 

            if (it == *this) break;

            for (; oct < 8; ++oct) 
            { 
                if ((*it).isBranch(oct))
                {
                    it = typename Tree::iterator(mTree, idx).goToDeapestPossition();
                    oct = 0;
                    break;
                }
            }   
        }
    }

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
auto 
SubTree<Tree>::findFirstLeaf() const noexcept
{ 
    auto tmp = *this;

    for (auto i = 0; i < 8; ++i) 
    { 
        if ( tmp.isLeaf(i)) { return tmp.index(i);} 

        if ( tmp.isBranch(i)) 
        { 
            tmp.goIn(i);
            i = -1;
        } 
    }
    return id_type::IndexMask;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
auto 
SubTree<Tree>::findLastLeaf() const noexcept
{ 
    auto tmp = *this;

    for (auto i = 7; !(i & 8); --i) 
    { 
        if ( tmp.isLeaf(i)) { return tmp.index(i); } 

        if ( tmp.isBranch(i)) 
        { 
            tmp.goIn(i);
            i = 8;
        } 
    }
    return id_type::IndexMask;
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace =========================================================================================================