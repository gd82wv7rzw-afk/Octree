#pragma once

#include <concepts>
#include <type_traits>

namespace Octree {
namespace Implementation {

/// @brief Forward declaration of the tree identifier type.
///
/// @tparam ST Integral type used to represent the identifier.
template<class ST>
requires std::is_integral_v<ST> && (!std::is_same_v<ST, bool>)
class TreeID;

//--------------------------------------------------------------------------------------------------

/// @brief Forward declaration of the bit-field type.
///
/// @tparam ST Unsigned integral type used to represent the bit field.
template<class ST>
requires std::is_unsigned_v<ST> && (!std::is_same_v<ST, bool>)
class BitFeld;

//--------------------------------------------------------------------------------------------------

/// @brief Forward declaration of the buffer type.
///
/// @tparam C Underlying container type.
template<class C>
class Buffer;

//--------------------------------------------------------------------------------------------------

/// @brief Checks whether a type provides the required container interface.
///
/// The concept verifies the presence of standard container types and
/// the operations required by the Octree implementation.
///
/// @tparam Container Type to be checked.
template<class Container>
concept container_type = requires
(
    const Container constC,
    Container c,
    typename Container::value_type x,
    typename Container::const_iterator cit
)
{
    typename Container::size_type;
    typename Container::value_type;
    typename Container::reference;
    typename Container::const_reference;
    typename Container::pointer;
    typename Container::const_pointer;
    typename Container::iterator;
    typename Container::const_iterator;

    { constC.size() }  -> std::same_as<typename Container::size_type>;

    { c.begin() }      -> std::same_as<typename Container::iterator>;
    { c.end() }        -> std::same_as<typename Container::iterator>;
    { c.cbegin() }     -> std::same_as<typename Container::const_iterator>;
    { c.cend() }       -> std::same_as<typename Container::const_iterator>;

    { c.insert(cit, x) } -> std::same_as<typename Container::iterator>;
    { c.push_back(x) };

    { constC.begin() } -> std::same_as<typename Container::const_iterator>;
    { constC.end() }   -> std::same_as<typename Container::const_iterator>;
};

//--------------------------------------------------------------------------------------------------

/// @brief Forward declaration of the tree base class.
///
/// @tparam T Element type stored in the container.
/// @tparam TT Additional type parameter.
/// @tparam ST Integral type used for tree identifiers.
/// @tparam Container Container template used to store elements.
/// @tparam Allocator Allocator template used by the container.
template<
    class T,
    class TT,
    typename ST,
    template<class, class> class Container,
    template<class> class Allocator
>
requires
    container_type<Container<T, Allocator<T>>> &&
    std::is_integral_v<ST> &&
    (!std::is_same_v<ST, bool>)
class TreeBase;

//--------------------------------------------------------------------------------------------------

} // namespace Implementation
} // namespace Octree
