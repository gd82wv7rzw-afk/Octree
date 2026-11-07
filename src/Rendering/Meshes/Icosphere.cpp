
#include "Icosphere.h"
#include "Rendering/mtlm.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace simd;
using namespace std;

Icosphere::Icosphere(size_t n, bool canonical) 
: mNdivisions(n ? n : 1) 
, mCanonical(canonical)
{
    generateIndexOffsets();
    //generateUVPoints();
    //generateTriangles();
}
//----------------------------------------------------------------------------------------------------------------------

const int Icosphere::zone(const int vId) const {
    if(0 == vId) return 3;
    return vId / mNdivisions;
}
//----------------------------------------------------------------------------------------------------------------------

const size_t Icosphere::nV() const { 
    return 3 * mNdivisions + 1;
}
//----------------------------------------------------------------------------------------------------------------------

const double Icosphere::dV(const int vId) const { 
    auto zoneId = zone(vId);
    double ldv;
    double base = acos(1.0 / sqrt(5.0)) / M_PI;
    switch (zoneId) {
        case 0: ldv = base / mNdivisions; break;
        case 1: ldv = (1 - 2 * base) / mNdivisions; break;
        case 2: ldv = base / mNdivisions; break;
        default: ldv = base / mNdivisions;
    }
    return ldv;
}
//----------------------------------------------------------------------------------------------------------------------

const size_t Icosphere::nU( const int vId) const { 

    auto zoneId = zone(vId);
    switch (zoneId) {
        case 0: return vId * 5 + 1;
        case 1: return mNdivisions * 5 + 1;
        case 2: return ( nV() - vId - 1) * 5 + 1;
    }
    
    return 5;
}
//----------------------------------------------------------------------------------------------------------------------

const double Icosphere::du(const int vId) const {
    if (zone(vId) == 3) return 1.0 / nU(vId);
    return 1.0 / (nU(vId)-1);
}
//----------------------------------------------------------------------------------------------------------------------

const double Icosphere::offsetU(int vId) const {
    if( 0 == vId) return 0.1;
    auto zoneId = zone(vId);
    switch (zoneId) {
        case 0: return 0.0;

        case 1: return (vId - mNdivisions)*0.1 / mNdivisions ;

        case 2: return 0.1;
    }
    return 0.2;
}
//----------------------------------------------------------------------------------------------------------------------

const size_t Icosphere::nPhi(int vId, bool canonical) const {

    if (!canonical) return nU(vId);
    if (zone(vId) == 3) return 1;
    return nU(vId) - 1;
}
//----------------------------------------------------------------------------------------------------------------------

const double Icosphere::dPhi(int vId, bool canonical) const {

    if ( canonical && (zone(vId) == 3) ) return 0.0;    
    return M_PI * 2 * du(vId);
}
//----------------------------------------------------------------------------------------------------------------------

const double Icosphere::phi0(int vId) const {
    return offsetU(vId) * M_PI * 2;
}
//----------------------------------------------------------------------------------------------------------------------

vector<float3> Icosphere::generatePoints() const{

    vector<float3> points;
    const auto nVpos = nV();
    
    double theta = 0.0;

    for (int vId = 0; vId < nVpos; ++vId) {

        

        const auto nPhil = nPhi(vId, mCanonical);

        double3 r = {0,0, cos( theta )};
        r.x = sqrt(1.0 - r.z * r.z);
        r.xy = mtlm::rotation( phi0(vId)) * r.xy;

        const auto incrPhi = mtlm::rotation( dPhi(vId) );

        for (int phiId = 0; phiId < nPhil; ++phiId) {
            points.push_back( make_float3( r.x, r.y, r.z) );
            r.xy = incrPhi * r.xy;
        }

        theta += M_PI * dV(vId);
    }
    return points;
}
//----------------------------------------------------------------------------------------------------------------------

void Icosphere::fillPoints(MetalArray<float3>& points) const{
    auto hPoints = generatePoints();

    for (size_t i = 0; i < hPoints.size(); ++i) {
        points[i] = hPoints[i];
    }
}
//----------------------------------------------------------------------------------------------------------------------

vector<float2> Icosphere::generateUVPoints() const {
    
    vector<float2> uvs;
    const auto nVpos = nV();
    double v = 0.0;

    for (int vId = 0; vId < nVpos; ++vId) {

        const size_t nUl = nPhi(vId, mCanonical);   

        double u = offsetU(vId);
        double dul = du(vId);

        for (int uId = 0; uId < nUl; ++uId) { 
            uvs.push_back( make_float2(u, 1.0 - v) );
            u += dul;
        }

        v += dV(vId);
    }

    return uvs;
}
//----------------------------------------------------------------------------------------------------------------------

vector<uint32_t> Icosphere::generateTriangles(bool canonical) const
{
    vector<uint32_t> indices;

    auto wrap = [&](uint32_t& id, uint32_t offset, uint32_t count) {
        if (!canonical) return;
        if (id - offset == count) id = offset;
    };
    //--------------------------------------------------------------------------------

    auto wrapPair = [&](uint32_t& up0,
        uint32_t& up1,
        uint32_t upOffset,
        uint32_t upCount,
        uint32_t& down0,
        uint32_t& down1,
        uint32_t downOffset,
        uint32_t downCount)
    {
        wrap(up0,   upOffset,   upCount);
        wrap(up1,   upOffset,   upCount);
        wrap(down0, downOffset, downCount);
        wrap(down1, downOffset, downCount);
    };
    //--------------------------------------------------------------------------------

    auto tri = [&](uint32_t a, uint32_t b, uint32_t c) { indices.insert(indices.end(), {a, b, c}); };
    //--------------------------------------------------------------------------------

    // north cap
    for (uint32_t vId = 0; vId < mNdivisions; ++vId)
    {
        const uint32_t upOffset   = mIndexOffsets[vId];
        const uint32_t downOffset = mIndexOffsets[vId + 1];

        const uint32_t upCount   = nPhi(vId, canonical);
        const uint32_t downCount = nPhi(vId + 1, canonical);

        for (uint32_t side = 0; side < 5; ++side)
        {
            auto id0 = upOffset   + side * upCount   / 5;
            auto id1 = downOffset + side * downCount / 5;
            auto id2 = id1 + 1;
            auto id3 = id0 + 1;

            for (uint32_t j = 0; j < vId; ++j)
            {
                wrapPair(
                    id0, id3, upOffset, upCount,
                    id1, id2, downOffset, downCount
                );

                tri(id0, id1, id2);
                tri(id0, id2, id3);

                ++id0;
                ++id1;
                ++id2;
                ++id3;
            }

            wrapPair(
                id0, id3, upOffset, upCount,
                id1, id2, downOffset, downCount
            );

            tri(id0, id1, id2);
        }
    }

    // middle zone
    for (uint32_t vId = mNdivisions; vId < mNdivisions * 2; ++vId) {
        const uint32_t upOffset   = mIndexOffsets[vId];
        const uint32_t downOffset = mIndexOffsets[vId + 1];

        const uint32_t upCount   = nPhi(vId, canonical);
        const uint32_t downCount = nPhi(vId + 1, canonical);

        auto id0 = upOffset;
        auto id1 = downOffset;
        auto id2 = id1 + 1;
        auto id3 = id0 + 1;

        for (uint32_t i = 0; i < 5 * mNdivisions; ++i)
        {
            wrapPair(
                id0, id3, upOffset, upCount,
                id1, id2, downOffset, downCount
            );

            tri(id0, id1, id3);
            tri(id3, id1, id2);

            ++id0;
            ++id1;
            ++id2;
            ++id3;
        }
    }

    // south cap
    for (uint32_t vId = mNdivisions * 2; vId < mNdivisions * 3; ++vId) {
        const uint32_t upOffset   = mIndexOffsets[vId];
        const uint32_t downOffset = mIndexOffsets[vId + 1];

        const uint32_t upCount   = nPhi(vId, canonical);
        const uint32_t downCount = nPhi(vId + 1, canonical);

        for (uint32_t side = 0; side < 5; ++side)
        {
            auto id0 = upOffset   + side * upCount   / 5;
            auto id1 = downOffset + side * downCount / 5;
            auto id2 = id1 + 1;
            auto id3 = id0 + 1;

            const auto end = nV() - vId - 2;

            for (uint32_t j = 0; j < end; ++j)
            {
                wrapPair(
                    id0, id3, upOffset, upCount,
                    id1, id2, downOffset, downCount
                );

                tri(id0, id1, id3);
                tri(id3, id1, id2);

                ++id0;
                ++id1;
                ++id2;
                ++id3;
            }

            wrapPair(
                id0, id3, upOffset, upCount,
                id1, id2, downOffset, downCount
            );

            tri(id0, id1, id3);
        }
    }

    return indices;
}
//----------------------------------------------------------------------------------------------------------------------

void Icosphere::fillTriangles(MetalArray<uint32_t>& triangles, bool canonical) const {
    auto hTriangles = generateTriangles(canonical);
    for (size_t i = 0; i < hTriangles.size(); ++i) {
        triangles[i] = hTriangles[i];
    }
}
//----------------------------------------------------------------------------------------------------------------------

vector<uint32_t> Icosphere::generateEdges() const{

    vector<uint32_t> triAngleIds = generateTriangles(true);
    vector<uint32_t> edgeIDs;

    const auto lnV = nV() - 1;

    // Every non-horizontal edge appears exactly once
    // as the first edge of a triangle.
    for (size_t triAnglID = 0; triAnglID < triAngleIds.size(); triAnglID += 3) {
        edgeIDs.insert( edgeIDs.end(), {triAngleIds[triAnglID], triAngleIds[triAnglID+1]} );
    }

    // ring edges
    for (size_t vId = 1; vId < lnV; ++vId) {

        const uint32_t offset = mIndexOffsets[vId];
        const uint32_t end = offset + nPhi(vId, true) - 1;
        uint32_t id0 = offset;

        for (; id0 < end; ++id0) {
            edgeIDs.insert( edgeIDs.end(), {id0, id0 + 1} );
        }
        edgeIDs.insert( edgeIDs.end(), {id0, offset} );
    }

    return edgeIDs;
}
//----------------------------------------------------------------------------------------------------------------------

void Icosphere::fillEdges(MetalArray<uint32_t>& edges) const {
    auto hEdges = generateEdges();
    for (size_t i = 0; i < hEdges.size(); ++i) {
        edges[i] = hEdges[i];
    }
}
//----------------------------------------------------------------------------------------------------------------------

void Icosphere::generateIndexOffsets() {
    mIndexOffsets.push_back(0);

    const auto lnV = nV();

    for (size_t i = 1; i < lnV; ++i) {
        auto off = mIndexOffsets.back();
        off += nPhi( i - 1, mCanonical);
        mIndexOffsets.push_back(off);
    }
}
//----------------------------------------------------------------------------------------------------------------------
