#pragma once

#include "Octree/Implementation/References/BaseReference.hpp"
#include "Octree/Implementation/Traits/TreeTraits.hpp"

namespace Octree::Implementation::Iterator {

template<class Tree>
class LeafStream: public References::BaseReference< typename Traits::TreeTraits<Tree>::leaf_type >
{
    protected:

        using tree_type = Traits::TreeTraits<Tree>;
        using base_type = References::BaseReference< typename tree_type::leaf_type >;
        using base_type::data;
        
        using id_type = Tree::id_type;

    public:

        using size_type = id_type::size_type;
        using difference_type = id_type::difference_type;
        //--------------------------------------------------------------------------------------------------------------

        constexpr LeafStream( Tree* tree  = nullptr, size_type index = 0) noexcept;
        //--------------------------------------------------------------------------------------------------------------

        auto operator *()                                                                             { return *data();}
        auto operator->()       noexcept                                                              { return  data();}
        auto operator *() const                                                                       { return *data();}
        auto operator->() const noexcept                                                              { return  data();}
        //--------------------------------------------------------------------------------------------------------------

        size_type index() const noexcept                                                              { return mIndex; }
        //--------------------------------------------------------------------------------------------------------------

        LeafStream& operator ++ ()    noexcept;
        LeafStream& operator -- ()    noexcept;
        LeafStream  operator ++ (int) noexcept                                    {auto t = *this; ++(*this); return t;}
        LeafStream  operator -- (int) noexcept                                    {auto t = *this; --(*this); return t;}
        //--------------------------------------------------------------------------------------------------------------

    protected:

        constexpr auto* leaf(  size_type i)  const noexcept                 { return mTree->leafs()->getPointer(i);    }
        constexpr auto* leaf(  size_type i)        noexcept                 { return mTree->leafs()->getPointer(i);    }
        constexpr auto* branch( size_type i) const noexcept                 { return mTree->branches()->getPointer(i); }
        constexpr auto* branch( size_type i)       noexcept                 { return mTree->branches()->getPointer(i); }
        //--------------------------------------------------------------------------------------------------------------
    
    private:

        Tree*     mTree{nullptr};  
        size_type mIndex{     0};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
constexpr LeafStream<Tree>::LeafStream( Tree* tree, size_type index) noexcept:
    base_type(),
    mIndex(index), 
    mTree(tree) 
{ 
    if ( tree != nullptr) base_type::operator = (mIndex != id_type::IndexMask ? leaf(index) : nullptr );
}
//--------------------------------------------------------------------------------------------------------------

template<class Tree>
LeafStream<Tree>&        
LeafStream<Tree>::operator ++ () noexcept
{
    if (!bool(*this)) { return *this; }

    //auto parendId = data()->parendId();
    auto rootIdx  = data()->parentIndex();
    char oct      = data()->possitonInParent() + 1;
    auto root     = (rootIdx != id_type::IndexMask ? branch(rootIdx) : nullptr);

    while ( root != nullptr )
    {

        for(; oct < 8; ++oct)
        {
            if ( root->childId(oct) & id_type::BranchBit)
            {
                rootIdx = root->childId(oct) & id_type::IndexMask;
                root    = branch(rootIdx);
                oct     = -1;

                continue;
            }

            if ( root->childId(oct) & id_type::LeafBit)
            {
                mIndex = root->childId(oct) & id_type::IndexMask;
                data() = leaf(mIndex);
                return *this;
            }
        }
        rootIdx  = root->parentIndex();
        oct      = root->possitonInParent() + 1;
        root     = (rootIdx != id_type::IndexMask ? branch(rootIdx) : nullptr);
    }

    data() = nullptr;

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

template<class Tree>
LeafStream<Tree>&        
LeafStream<Tree>::operator -- () noexcept
{
    if (!bool(*this)) { return *this; }

    auto parendId = data()->parendId();
    auto rootIdx  = parendId & id_type::IndexMask;
    char oct      = id_type::OctantToNumber(parendId) - 1;
    auto root     = (rootIdx != id_type::IndexMask ? branch(rootIdx) : nullptr );

    while ( root != nullptr) 
    {
        for(; !(oct & 8); --oct)
        {
            if ( root->childId(oct) & id_type::BranchBit)
            {
                rootIdx = root->childId(oct) & id_type::IndexMask;
                root    = branch(rootIdx);
                oct     = 8;

                continue;
            }

            if ( root->childId(oct) & id_type::LeafBit)
            {
                mIndex = root->childId(oct) & id_type::IndexMask;
                data() = leaf(mIndex);
                return *this;
            }
        }

        parendId = root->parendId();
        rootIdx  = parendId & id_type::IndexMask;
        oct      = id_type::OctantToNumber(parendId) - 1;
        root     = ( rootIdx != id_type::IndexMask ? branch(rootIdx) : nullptr );
    } 

    data() = nullptr;

    return *this;
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace =========================================================================================================