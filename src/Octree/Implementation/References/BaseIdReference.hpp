#pragma once

#include "Octree/Implementation/References/BaseReference.hpp"
#include "Octree/Implementation/TreeID.hpp"

namespace Octree::Implementation::References {
    
template<class ST>
class BaseIdReference: public BaseReference<ST>
{
    protected:

        using base_type = References::BaseReference<ST>;
        using id_type   = TreeID<ST>;
        using typename base_type::Pointer;
        //--------------------------------------------------------------------------------------------------------------

    public:

        explicit constexpr BaseIdReference(Pointer data) noexcept: base_type(data) {}
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto index()                                             {return id_type::Index(*base_type::data()); }
        constexpr auto index() const                                       {return id_type::Index(*base_type::data()); }
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto octant()                                            {return id_type::Octant(*base_type::data());}
        constexpr auto octant() const                                      {return id_type::Octant(*base_type::data());}
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace =========================================================================================================