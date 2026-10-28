#pragma once

#include "Octree/Implementation/References/BaseReference.hpp"

#include <type_traits>

namespace Octree {
namespace Implementation {
namespace References{

template<class ST, auto Mask>
requires std::is_integral_v<ST> && (!std::is_same_v<ST, bool>)
class IdReference: public BaseReference<ST>
{
    protected:
        
        template<class T, auto M> 
        requires std::is_integral_v<T> && (!std::is_same_v<T, bool>)
        friend class IndexReference;
        //--------------------------------------------------------------------------------------------------------------

        using base_type = BaseReference<ST>;
        using base_type::isConst;
        //--------------------------------------------------------------------------------------------------------------
    
    public:

        using typename base_type::value_type;
        //--------------------------------------------------------------------------------------------------------------

        constexpr IdReference(base_type::Pointer data) noexcept: base_type(data)                                      {}
        constexpr IdReference()                        noexcept: base_type(nullptr)                                   {}
        //--------------------------------------------------------------------------------------------------------------

        constexpr operator value_type () const noexcept                             {return Mask & *base_type::data(); }

        constexpr IdReference& operator  = (value_type x) noexcept requires (!isConst )
                                       { *base_type::data() = (x & Mask) | (*base_type::data() & ~Mask); return *this; }
                                       
        constexpr IdReference& operator |= (value_type x) noexcept requires (!isConst ) 
                                                                   { return this->operator=(*base_type::data() | x ) ; }

        constexpr IdReference& operator &= (value_type x) noexcept requires (!isConst ) 
                                                                   { return this->operator=(*base_type::data() | x ) ; }
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace References 
} // namespace Implementation 
} // namespace Octree ==================================================================================================