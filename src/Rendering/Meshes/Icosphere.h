#pragma once

#include "MetalStuff/MetalArray.hpp"


#include <cstddef>
#include <cstdint>
#include <vector>
#include <simd/simd.h>

/// @brief Generates an icosphere parameterization and mesh topology.
class Icosphere {
public:

    /// @brief Constructs an icosphere descriptor.
    /// @param nDevisions Number of subdivisions per icosahedron edge.
    /// @param nDevisions Number of edge segments per icosahedron edge.
    ///
    /// Examples:
    /// - 1 : original icosahedron (no subdivision)
    /// - 2 : edge split once
    /// - 3 : edge split into three segments
    ///
    /// In general:
    /// subdivisions = nDevisions - 1
    ///
    /// @param isCanonical If true, only canonical points are generated.
    ///                    If false, periodic image points are included.
    Icosphere(size_t nDevisions = 1, bool isCanonical = true);
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the theta zone of a v ring.
    ///
    /// Zones:
    /// - 0 : 0° + ε .. 60°
    /// - 1 : 60° .. 120°
    /// - 2 : 120° .. 180° - ε
    /// - 3 : poles
    ///
    /// @param vId V-ring index.
    /// @return Zone identifier.
    const int zone(const int vId) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the number of v samples.
    /// @return Number of v points.
    const size_t nV() const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the spacing between two v samples.
    /// @return V interval.
    const double dV(const int vId) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the number of u samples on a v ring.
    /// @param vId V-ring index.
    /// @return Number of u points on the ring.
    const size_t nU(const int vId) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the angular spacing of u samples on a v ring.
    /// @param vId V-ring index.
    /// @return U interval on the ring.
    const double du(const int vId) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the u offset for a v ring.
    /// @param vId V-ring index.
    /// @return U offset of the ring.
    const double offsetU(int vId) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the number of phi samples on a v ring.
    /// @param vId V-ring index.
    /// @param canonical If true, only canonical samples are counted.
    /// @return Number of phi samples.
    const size_t nPhi(int vId, bool canonical = true) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the starting phi angle of a v ring.
    /// @param vId V-ring index.
    /// @return Initial phi angle.
    const double phi0(int vId) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Returns the phi interval of a v ring.
    /// @param vId V-ring index.
    /// @param canonical If true, canonical spacing is used.
    /// @return Phi interval.
    const double dPhi(int vId, bool canonical = true) const;
    //-------------------------------------------------------------------------------------------------------------------

    inline const size_t size() const {return mIndexOffsets.back() + nU(nV() - 1);}
    //-------------------------------------------------------------------------------------------------------------------

    inline const size_t nEdges() const { return 30 * mNdivisions * mNdivisions; }
    //-------------------------------------------------------------------------------------------------------------------

    inline const size_t nTriangles() const { return 20 * mNdivisions * mNdivisions; }

    /// @brief Generates all 3D sphere vertices.
    /// @return Array of sphere positions.
    std::vector<simd::float3> generatePoints() const;
    void fillPoints(MetalArray<simd::float3>& points) const;
    //MetalArray<simd::float3> generatePoints(MTL::Device* device);
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Generates UV coordinates for all vertices.
    /// @return Array of UV coordinates.
    std::vector<simd::float2> generateUVPoints() const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Generates triangle index data.
    /// @param canonical If true, generates canonical topology only.
    /// @return Triangle index buffer.
    std::vector<uint32_t> generateTriangles(bool canonical) const;
    void fillTriangles(MetalArray<uint32_t>& triangles, bool canonical) const;
    //-------------------------------------------------------------------------------------------------------------------

    /// @brief Generates edge index data.
    /// @return Edge index buffer containing vertex index pairs.
    std::vector<uint32_t> generateEdges() const;
    void fillEdges(MetalArray<uint32_t>& edges) const;
    //-------------------------------------------------------------------------------------------------------------------

    const std::vector<size_t>& indexOffsets() const {return mIndexOffsets;}

private:

    /// @brief Generates index offsets for all v rings.
    void generateIndexOffsets();

    /// @brief Number of subdivisions per icosahedron edge.
    size_t mNdivisions = 1;

    /// @brief Canonical point generation mode.
    ///
    /// If true, only canonical points are generated.
    /// If false, periodic image points are included.
    bool mCanonical = true;

    /// @brief Prefix offsets for ring indexing.
    std::vector<size_t> mIndexOffsets;
};
//----------------------------------------------------------------------------------------------------------------------