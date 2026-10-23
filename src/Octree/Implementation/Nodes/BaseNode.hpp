#pragma once

#include "Octree/Implementation/Nodes/Nodes.h"
#include <type_traits>

namespace Octree::Implementation 
{
    template<class C>
    class Buffer;

    template<class T>
    class MetalBuffer;
}
//----------------------------------------------------------------------------------------------------------------------

namespace Octree::Implementation::References 
{
    template<class Tree>
    class SubTree;
}
//----------------------------------------------------------------------------------------------------------------------

namespace Octree::Implementation::Iterator
{
    template<class Tree>
    class BaseStream;
}
//----------------------------------------------------------------------------------------------------------------------

namespace Octree::Implementation::Iterator
{
    template<class Tree>
    class LeafStream;
}
//----------------------------------------------------------------------------------------------------------------------


namespace Octree::Implementation::Nodes {

template<class T>
class Value
{    
    public:
    
        using value_type  = T;
        //--------------------------------------------------------------------------------------------------------------
  
        operator value_type& () const noexcept { return mData; }
        Value& operator = (const value_type& value) noexcept { mData = value; return *this; }
        //--------------------------------------------------------------------------------------------------------------

    protected:

        T   mData{};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

template<>
class Value<void>
{    
    public:
    
        using value_type  = void;
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------


template<class T, class ID>
class BaseNode: public Value<T>
{
    protected:

        using base_type = Value<T>;
        //--------------------------------------------------------------------------------------------------------------
    
        template<class C>
        friend class Implementation::Buffer;

        template<class C>
        friend class Implementation::MetalBuffer;

        template<class Tree>
        friend class References::SubTree;

        template<class C1, class C2>
        friend class BaseTree;

        template<class Tree>
        friend class Iterator::BaseStream;

        template<class Tree>
        friend class Iterator::LeafStream;
        //--------------------------------------------------------------------------------------------------------------

    public:

        using size_type              = ID::size_type;
        using difference_type        = ID::difference_type;
        //--------------------------------------------------------------------------------------------------------------

        auto& value() const noexcept requires (!std::is_void_v<T>)                               { return this->mData; }
        auto& value()       noexcept requires (!std::is_void_v<T>)                               { return this->mData; }
        //--------------------------------------------------------------------------------------------------------------

        static constexpr auto NodeTyp()                noexcept                 { return NodeType<ID>::Type::LeafNode; }
        constexpr        auto parentIndex()      const noexcept                 { return mId & ID::IndexMask;          }
        constexpr        auto possitonInParent() const noexcept                 { return ID::OctantToNumber(mId);      }
        //--------------------------------------------------------------------------------------------------------------

    protected:
 
        auto  parendId() const noexcept { return mId; }
        auto& parendId()       noexcept { return mId; }
        //--------------------------------------------------------------------------------------------------------------

    private:

        size_type mId{};
        //Value<T>  mValue{};
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace ========================================================================================================