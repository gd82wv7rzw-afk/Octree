#pragma once

#include "Octree/Implementation/References/BaseReference.hpp"

namespace Octree::Implementation::References{

template<class Branch>
class ParentIdReference: public BaseReference<Branch>
{
    protected:

        using base_type = BaseReference<Branch>;
        using typename base_type::Pointer;
        using id_type = Branch::id_type;
        using base_type::isConst;
        //--------------------------------------------------------------------------------------------------------------

    public:

        explicit constexpr ParentIdReference(Pointer data) noexcept: base_type(data) {}
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto index() const                                    {return id_type::Index(this->data()->mParend); }
        constexpr auto index() const requires (!isConst)                {return id_type::Index(this->data()->mParend); }
        //--------------------------------------------------------------------------------------------------------------

        constexpr auto octant() const                                   {return id_type::Octant(this->data()->mParend);}
        constexpr auto octant() const requires (!isConst)               {return id_type::Octant(this->data()->mParend);}
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace =========================================================================================================