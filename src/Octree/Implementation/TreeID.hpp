#pragma once

#include "Octree/Implementation/References/IdReference.hpp"

#include <concepts>
#include <type_traits>

namespace Octree::Implementation {

/// @brief Encodes an octree identifier using index and octant bits.
///
/// The identifier reserves the three most significant bits of the underlying
/// unsigned type for octant information, branch identification, and deletion
/// status. The remaining bits represent the element index.
///
/// @tparam ST Integral type used to define the identifier's bit width.
///            Boolean types are not supported.
template<class ST>
requires std::is_integral_v<ST> && (!std::is_same_v<ST, bool>)
struct TreeID
{
    /// @brief Unsigned type used to store identifiers.
    using size_type = std::make_unsigned_t<ST>;

    /// @brief Signed type used to represent index differences.
    using difference_type = std::make_signed_t<ST>;

    //--------------------------------------------------------------------------------------------------

    /// @brief Bit position of the first reserved octant bit.
    static constexpr const size_type OctantOffsetBitNumber = 8 * sizeof(size_type) - 3;

    /// @brief Mask selecting the three reserved high-order bits.
    static constexpr const size_type OctantMask = 0b111 << OctantOffsetBitNumber;

    /// @brief Mask selecting the index bits.
    static constexpr const size_type IndexMask = ~OctantMask;

    /// @brief Bit indicating a branch node.
    static constexpr const size_type BranchBit = 0b100 << OctantOffsetBitNumber;

    /// @brief Bit indicating a leaf node.
    static constexpr const size_type LeafBit = 0b010 << OctantOffsetBitNumber;

    /// @brief Bit indicating a deleted entry.
    static constexpr const size_type DeletedBit = 0b001 << OctantOffsetBitNumber;
    //--------------------------------------------------------------------------------------------------

    /// @brief Reference proxy for accessing or modifying the index bits.
    ///
    /// @tparam T Either size_type or difference_type.
    template<class T>
    requires std::same_as<T, size_type> || std::same_as<T, difference_type>
    using index_reference = References::IdReference<T, IndexMask>;

    /// @brief Const reference proxy for accessing the index bits.
    using const_index_reference = References::IdReference<const size_type, IndexMask>;
    //--------------------------------------------------------------------------------------------------

    /// @brief Reference proxy for accessing or modifying the octant bits.
    ///
    /// @tparam U Either size_type or difference_type.
    template<class U>
    requires std::same_as<U, size_type> || std::same_as<U, difference_type>
    using octant_reference = References::IdReference<U, OctantMask>;

    /// @brief Const reference proxy for accessing the octant bits.
    using const_octant_reference = References::IdReference<const size_type, OctantMask>;
    //--------------------------------------------------------------------------------------------------

    /// @brief Returns a proxy for accessing or modifying the index bits.
    ///
    /// @tparam T Either size_type or difference_type.
    /// @param data Reference to the encoded identifier.
    /// @return Proxy referencing the index bits.
    template<class T>
    requires std::same_as<T, size_type> ||
             std::same_as<T, difference_type>
    static constexpr auto Index(T& data)
    {
        return index_reference<T>(&data);
    }

    /// @brief Extracts the index bits from an identifier.
    ///
    /// @param data Encoded identifier.
    /// @return The index portion of the identifier.
    static constexpr auto Index(size_type data)
    {
        return data & IndexMask;
    }

    //--------------------------------------------------------------------------------------------------

    /// @brief Returns a proxy for accessing or modifying the octant bits.
    ///
    /// @tparam U Either size_type or difference_type.
    /// @param data Reference to the encoded identifier.
    /// @return Proxy referencing the octant bits.
    template<class U>
    requires std::same_as<U, size_type> || std::same_as<U, difference_type>
    static constexpr auto Octant(U& data) {return octant_reference<U>(&data);}

    /// @brief Extracts the octant bits from an identifier.
    ///
    /// @param data Reference to the encoded identifier.
    /// @return The octant portion of the identifier.
    static constexpr auto Octant(size_type& data){ return data & OctantMask; }
    //--------------------------------------------------------------------------------------------------

    /// @brief Encodes an octant number into its bit-field position.
    ///
    /// @param data Octant number.
    /// @return The octant number shifted to the reserved bit positions.
    static constexpr auto NumberToOctant(size_type data){return data << OctantOffsetBitNumber;}

    /// @brief Extracts an octant number from its encoded bit-field.
    ///
    /// @param data Encoded octant value.
    /// @return The octant number shifted back to the least significant bits.
    static constexpr auto OctantToNumber(size_type data){return data >> OctantOffsetBitNumber;}
    //--------------------------------------------------------------------------------------------------

    /// @brief Stores an encoded octree identifier.
    class Id
    {
    private:

        /// @brief Raw encoded identifier value.
        size_type mData{};
        //--------------------------------------------------------------------------------------------------

    public:

        /// @brief Constructs an identifier from a raw encoded value.
        ///
        /// @param data Initial encoded value. Defaults to zero.
        constexpr Id(size_type data = 0) noexcept
            : mData(data)
        {
        }
        //--------------------------------------------------------------------------------------------------

        /// @brief Returns a proxy for accessing the index bits.
        constexpr auto index() const noexcept
        {
            return Index(mData);
        }

        /// @brief Returns a proxy for accessing or modifying the index bits.
        constexpr auto index() noexcept
        {
            return Index(mData);
        }

        /// @brief Returns the octant bits.
        constexpr auto octant() const noexcept
        {
            return Octant(mData);
        }

        /// @brief Returns a proxy for accessing or modifying the octant bits.
        constexpr auto octant() noexcept
        {
            return Octant(mData);
        }

        /// @brief Sets the octant from an octant number.
        ///
        /// @param number Octant number to encode.
        /// @return Reference to this identifier.
        constexpr auto& numberToOctant(size_type number) noexcept
        {
            Octant(mData) = NumberToOctant(number);
            return *this;
        }

        /// @brief Converts the stored octant bits to an octant number.
        ///
        /// @return Decoded octant number.
        constexpr auto octantToNumber() const noexcept
        {
            return OctantToNumber(mData);
        }

        /// @brief Converts the identifier to its raw unsigned value.
        constexpr explicit operator size_type() const noexcept
        {
            return mData;
        }
        //--------------------------------------------------------------------------------------------------
    };

    //--------------------------------------------------------------------------------------------------
};

} // namespace Octree::Implementation
