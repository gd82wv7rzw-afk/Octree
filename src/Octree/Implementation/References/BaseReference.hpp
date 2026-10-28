#pragma once

#include "Octree/Implementation/References/References.h"
#include <cassert>
#include <type_traits>

namespace Octree {
namespace Implementation {
namespace References{

template<class T>
class BaseReference
{
    protected:

        template <class> friend class BaseReference;
        static constexpr const bool isConst = std::is_const_v<T>; 
        using Pointer = std::conditional_t<isConst, const T*, T*>;   
        //--------------------------------------------------------------------------------------------------------------

    public:

        using value_type      = T;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;
        //--------------------------------------------------------------------------------------------------------------

        constexpr explicit BaseReference(Pointer data) noexcept: mRoot(data)                                          {}
        constexpr          BaseReference()             noexcept: mRoot(nullptr)                                       {}
        //--------------------------------------------------------------------------------------------------------------

        template<class OT> 
        BaseReference(const BaseReference<OT>& o) noexcept requires(isConst && !std::is_const_v<OT>): mRoot(o.mRoot)  {}
        //--------------------------------------------------------------------------------------------------------------

        constexpr explicit operator bool() const noexcept                                   { return mRoot != nullptr; }
        //--------------------------------------------------------------------------------------------------------------

        friend constexpr bool operator == (BaseReference const& o, std::nullptr_t) noexcept {return o.mRoot == nullptr;}
        friend constexpr bool operator != (BaseReference const& o, std::nullptr_t) noexcept {return o.mRoot != nullptr;}
        friend constexpr bool operator == (std::nullptr_t, BaseReference const& o) noexcept {return o.mRoot == nullptr;}
        friend constexpr bool operator != (std::nullptr_t, BaseReference const& o) noexcept {return o.mRoot != nullptr;}
        //--------------------------------------------------------------------------------------------------------------

        constexpr bool operator == (const BaseReference& o) const noexcept                  { return mRoot == o.mRoot; }
        constexpr bool operator != (const BaseReference& o) const noexcept                  { return mRoot != o.mRoot; }
        //--------------------------------------------------------------------------------------------------------------

        constexpr BaseReference& operator = ( Pointer ptr )       noexcept            { mRoot = ptr;     return *this; }
        constexpr BaseReference& operator = ( std::nullptr_t)     noexcept            { mRoot = nullptr; return *this; }
        //--------------------------------------------------------------------------------------------------------------
    
    protected:

        auto& data()       noexcept                                              {                    return mRoot; }
        auto& data() const noexcept                                              { assert(bool(*this) ); return mRoot; }
        //--------------------------------------------------------------------------------------------------------------

    private:

        Pointer mRoot{nullptr};
        //--------------------------------------------------------------------------------------------------------------
};

} // namespace References 
} // namespace Implementation 
} // namespace Octree ==================================================================================================