#pragma once

#include <type_traits>

namespace Octree {
namespace Implementation {
namespace References{

template<class T>
class BaseReference;
//----------------------------------------------------------------------------------------------------------------------

template<class ST, auto Mask>
requires std::is_integral_v<ST> && (!std::is_same_v<ST, bool>)
class IdReference;
//----------------------------------------------------------------------------------------------------------------------

template<class Branch>
class ParentIdReference;
//----------------------------------------------------------------------------------------------------------------------

template<class ST>
class BaseIdReference;
//----------------------------------------------------------------------------------------------------------------------

} // namespace References 
} // namespace Implementation ==========================================================================================
} // namespace Octree ==================================================================================================