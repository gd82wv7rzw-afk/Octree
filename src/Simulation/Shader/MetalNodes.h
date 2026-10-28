#pragma once

#include "Simulation/Atom.h"
#include "Simulation/Branch.h"

namespace Simulation {

enum : uint 
{
    /// @brief Bit position of the first reserved octant bit.
    OctantOffsetBitNumber = 8 * sizeof(uint) - 3,
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Mask selecting the three reserved high-order bits.
    OctantMask = 0b111u << OctantOffsetBitNumber,
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Mask selecting the index bits.
    IndexMask = ~OctantMask,
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Bit indicating a branch node.
    BranchBit = 0b100u << OctantOffsetBitNumber
    //------------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

struct LeafNode 
{
    Atom value;
    uint rootId;
};
//----------------------------------------------------------------------------------------------------------------------

struct BranchNode 
{
    Branch value;
    uint rootId;
    uint childIds[8];
    uint childSizes[8];
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace Simulation