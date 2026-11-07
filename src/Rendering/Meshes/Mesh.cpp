#include "Mesh.h"
#include "Icosphere.h"

#include <Metal/Metal.hpp>
#include <cmath>
#include <simd/simd.h>
#include <utility>
#include <vector>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>

#include <cstdio>

Mesh::~Mesh()
{
    // Release vertex buffer if it exists
    if (vertexBuffer)
    {
        vertexBuffer->release();
        vertexBuffer = nullptr;
    }

    // Release index buffer if it exists
    if (indexBuffer)
    {
        indexBuffer->release();
        indexBuffer = nullptr;
    }
}
//----------------------------------------------------------------------------------------------------------------------

namespace {
// Small epsilon used for floating-point comparisons to avoid precision issues
constexpr float NUM_ZERO = 0.00001f;
//----------------------------------------------------------------------------------------------------------------------

using namespace std;
using namespace simd;

// Default GPU vertex type used by the rendering pipeline (position, normal, optional UV/color)
//using Vertex = Vertices::PNVertex;
//----------------------------------------------------------------------------------------------------------------------

// CPU-side raw mesh containing only geometry data (positions + CCW triangle indices), no GPU data
struct RawMesh {
    std::vector<simd::float3> vertices;
    std::vector<uint32_t> indices;
};
//----------------------------------------------------------------------------------------------------------------------

/// @brief add an triangle to an existing mesh
void addTriangleToRawMesh(const RawMesh& triangle, RawMesh& targetMesh) {
    for (int i = 0; i < 3; ++i) {
        
        // we have alredy the vertex
        auto id = triangle.indices[i];
        if (id < targetMesh.vertices.size()) {
            targetMesh.indices.push_back(id);
        }
        else {
            // we add the new vertex
            id = targetMesh.vertices.size();
            targetMesh.vertices.push_back(triangle.vertices[i]);
            targetMesh.indices.push_back(id);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------

/// @brief return uv, if |r| < epsilon zero return the origen
float2 sphereCoortToUV(const float3& pos) {

    // if |r| < epsilon zero return the origen
    float abs = dot(pos, pos);
    if (abs < NUM_ZERO) return {0.5, 0.5};

    // project the surface of the sphere to the uv space
    auto r = pos / sqrtf(abs);
    float u = acosf(r.z) / M_PI;
    float v = (atan2f(r.y, r.x) + M_PI) / (M_PI * 2);

    // and return u,v
    return {u,v};
}
//----------------------------------------------------------------------------------------------------------------------

/// @brief add an triangle to an existing uv grit
void addTriangleToUV(const RawMesh& triangle, vector<float2>& targetUV, float2(*creatUV)(const float3& pos)) {
    float2 tmpUV = {0,0};
    float minu = 1;
    float maxu = 0;

    for (int i = 0; i < 3; ++i) {
        
        // we have alredy the uv
        auto id = triangle.indices[i];
        if (id < targetUV.size()) { continue;}

        else {
            id = targetUV.size();
            tmpUV = creatUV(triangle.vertices[i]);

            // seam check
            if(tmpUV.x > maxu) maxu = tmpUV.x;
            if (tmpUV.x < minu) minu = tmpUV.x;

            targetUV.push_back(tmpUV);
        }
    }

    // we have to corecct a splitted triangle
    if ((maxu - minu) > 0.5) {
        for (int i = 0; i < 3; ++i) {

            auto id = triangle.indices[i];
            if ((targetUV[id].x - minu) < 0.5 ) targetUV[id].x -= 1;
        }   
    }
}
//----------------------------------------------------------------------------------------------------------------------

RawMesh createCubeRaw()
{
    RawMesh mesh;

    mesh.vertices = {
        simd::float3{-1,-1, 1},
        simd::float3{ 1,-1, 1},
        simd::float3{ 1, 1, 1},
        simd::float3{-1, 1, 1},
        simd::float3{-1,-1,-1},
        simd::float3{ 1,-1,-1},
        simd::float3{ 1, 1,-1},
        simd::float3{-1, 1,-1}
    };

    mesh.indices = {
        // front
        0,1,2, 2,3,0,

        // back
        5,4,7, 7,6,5,

        // left
        4,0,3, 3,7,4,

        // right
        1,5,6, 6,2,1,

        // top
        3,2,6, 6,7,3,

        // bottom
        4,5,1, 1,0,4
    };

    return mesh;
}
//----------------------------------------------------------------------------------------------------------------------

simd::float3 computeFaceNormal(const simd::float3& v0, const simd::float3& v1, const simd::float3& v2)
{
    simd::float3 e1 = v1 - v0;
    simd::float3 e2 = v2 - v0;

    simd::float3 n =
    {
        e1.y * e2.z - e1.z * e2.y,
        e1.z * e2.x - e1.x * e2.z,
        e1.x * e2.y - e1.y * e2.x
    };

    float len2 = n.x*n.x + n.y*n.y + n.z*n.z;

    if (NUM_ZERO < len2)
    {
        float invLen = 1.0f / std::sqrt(len2);
        n *= invLen;
    }
    else
    {
        n = {0,0,0};
    }

    return n;
}
//-------------------------------------------------------------------------------------------------------------------

std::vector<simd::float3>  calculateNormals(const RawMesh& inMesh) {
    
    const size_t vcount = inMesh.vertices.size();
    const size_t icount = inMesh.indices.size();

    // inicialice normal container with 0 vector
    std::vector<simd::float3> normal(vcount, simd::float3{0,0,0});

    for (size_t i = 0; i < icount; i += 3) {
        auto i0 = inMesh.indices[i];
        auto i1 = inMesh.indices[i + 1];
        auto i2 = inMesh.indices[i + 2];

        const simd::float3& v0 = inMesh.vertices[i0];
        const simd::float3& v1 = inMesh.vertices[i1];
        const simd::float3& v2 = inMesh.vertices[i2];

        auto n = computeFaceNormal(v0, v1, v2);

        normal[i0] += n;
        normal[i1] += n;
        normal[i2] += n;
    }

    // final normalization (IMPORTANT)
    for (auto& n : normal)
    {
        // "invLen" is used as temporary length first, then converted to inverse for normalization (performance-local reuse pattern)
        float invLen = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);

        if (NUM_ZERO < invLen) invLen = 1.0 / invLen;
        else invLen = 0.0;

        n *= invLen;
    }

    return normal;
}
//-------------------------------------------------------------------------------------------------------------------

// TODO wir verdoppeln zu viele vertices
// WARNING: mutates input mesh (topology expansion)
std::vector<simd::float3> buildFlatShadedMeshAndNormals(RawMesh& inMesh)
{
    std::vector<simd::float3> outNormals;
    outNormals.reserve(inMesh.indices.size());

    std::vector<simd::float3> newPositions;
    newPositions.reserve(inMesh.indices.size());

    std::vector<uint32_t> newIndices;
    newIndices.reserve(inMesh.indices.size());

    for (size_t i = 0; i < inMesh.indices.size(); i += 3)
    {
        uint32_t i0 = inMesh.indices[i + 0];
        uint32_t i1 = inMesh.indices[i + 1];
        uint32_t i2 = inMesh.indices[i + 2];

        const simd::float3& v0 = inMesh.vertices[i0];
        const simd::float3& v1 = inMesh.vertices[i1];
        const simd::float3& v2 = inMesh.vertices[i2];

        // Face normal (CCW assumed)
        simd::float3 n = computeFaceNormal(v0, v1, v2);

        // Vertex expansion (3 new vertices per face)
        uint32_t baseIndex = static_cast<uint32_t>(newPositions.size());

        newPositions.push_back(v0);
        newPositions.push_back(v1);
        newPositions.push_back(v2);

        // Flat normals (duplicated per vertex)
        outNormals.push_back(n);
        outNormals.push_back(n);
        outNormals.push_back(n);

        // CCW index winding
        newIndices.push_back(baseIndex + 0);
        newIndices.push_back(baseIndex + 1);
        newIndices.push_back(baseIndex + 2);
    }

    // In-place overwrite (explicit design decision)
    inMesh.vertices = std::move(newPositions);
    inMesh.indices  = std::move(newIndices);

    return outNormals;
}
//-------------------------------------------------------------------------------------------------------------------

// TODO Anpassen wenn flat shading repariert ist
simd::float2 calculateCubeUVs(const simd::float3& pos)
{
    static int n = -1;
    ++n;
    n %= 6;
    
    switch (n) {
        case 0: return {0,0};
        case 1: return {1,0};
        case 2: return {1,1};
        case 3: return {1,1};
        case 4: return {0,1};
        case 5: return {0,0};
    }
    return {-1,-1};
}
//-------------------------------------------------------------------------------------------------------------------

std::vector<simd::float2> buildUVs( const RawMesh& inMesh, simd::float2(*calcUV)(const simd::float3& pos)) 
{
    std::vector<simd::float2> outUVs;
    outUVs.resize(inMesh.vertices.size());
    for (size_t i = 0; i < inMesh.vertices.size(); ++i) outUVs[i] = calcUV(inMesh.vertices[i]);
    return outUVs;
}
//-------------------------------------------------------------------------------------------------------------------

uint32_t getMidpoint(
    const RawMesh& inMesh,
    RawMesh& outMesh,
    std::unordered_map<uint64_t, uint32_t>& midpointCache,
    uint32_t a,
    uint32_t b)
{
    uint32_t minIdx = std::min(a, b);
    uint32_t maxIdx = std::max(a, b);

    uint64_t key = (static_cast<uint64_t>(minIdx) << 32) | maxIdx;

    auto it = midpointCache.find(key);
    if (it != midpointCache.end()) return it->second;

    simd::float3 mid = (inMesh.vertices[a] + inMesh.vertices[b]) * 0.5f;

    uint32_t newIndex = static_cast<uint32_t>(outMesh.vertices.size());
    outMesh.vertices.push_back(mid);

    midpointCache[key] = newIndex;

    return newIndex;
}
//-------------------------------------------------------------------------------------------------------------------

RawMesh subdivideRawMesh(const RawMesh& inMesh)
{
    RawMesh out;
    out.vertices = inMesh.vertices;

    std::unordered_map<uint64_t, uint32_t> midpointCache;

    for (size_t i = 0; i < inMesh.indices.size(); i += 3)
    {
        uint32_t i0 = inMesh.indices[i + 0];
        uint32_t i1 = inMesh.indices[i + 1];
        uint32_t i2 = inMesh.indices[i + 2];

        uint32_t a = getMidpoint(inMesh, out, midpointCache, i0, i1);
        uint32_t b = getMidpoint(inMesh, out, midpointCache, i1, i2);
        uint32_t c = getMidpoint(inMesh, out, midpointCache, i2, i0);

        out.indices.insert(out.indices.end(), {
            i0, a, c,
            i1, b, a,
            i2, c, b,
            a,  b, c
        });
    }

    return out;
}
//-------------------------------------------------------------------------------------------------------------------

void projectToSphere(RawMesh& mesh, float radius)
{
    for (auto& vertex : mesh.vertices)
    {
        vertex = simd_normalize(vertex) * radius;
    }
}
//-------------------------------------------------------------------------------------------------------------------

struct VertexAttributes
{
    std::vector<simd::float3> normals;
    std::vector<simd::float2> uvs;
    std::vector<simd::float3> colors;
};
//-------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> finalizePN(
    MTL::Device* device,
    const RawMesh& rawMesh,
    const std::vector<simd::float3>& normals) 
{
    auto mesh = std::make_unique<Mesh>();
    using Vertex = Rendering::PNVertex;
    std::vector<Vertex> vertices;
    vertices.reserve(rawMesh.vertices.size());

    // Build GPU vertex buffer (position + normal)
    for (size_t i = 0; i < rawMesh.vertices.size(); ++i)
    {
        Vertex v;
        v.position = rawMesh.vertices[i];
        v.normal   = normals[i];

        vertices.push_back(v);
    }

    mesh->vertexBuffer = device->newBuffer(
        vertices.data(),
        vertices.size() * sizeof(Vertex),
        MTL::ResourceStorageModeShared
    );

    mesh->indexBuffer = device->newBuffer(
        rawMesh.indices.data(),
        rawMesh.indices.size() * sizeof(uint32_t),
        MTL::ResourceStorageModeShared
    );

    mesh->vertexCount = static_cast<uint32_t>(vertices.size());
    mesh->indexCount  = static_cast<uint32_t>(rawMesh.indices.size());

    return mesh;
}
//-------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> finalizePNT(
    MTL::Device* device,
    const RawMesh& rawMesh,
    const std::vector<simd::float3>& normals,
    const std::vector<simd::float2>& uvs) 
{
    auto mesh = std::make_unique<Mesh>();
    using Vertex = Rendering::PNTVertex;
    std::vector<Vertex> vertices;
    vertices.reserve(rawMesh.vertices.size());

    // Build GPU vertex buffer (position + normal)
    for (size_t i = 0; i < rawMesh.vertices.size(); ++i)
    {
        Vertex v;
        v.position = rawMesh.vertices[i];
        v.normal   = normals[i];
        v.texture  = uvs[i];

        vertices.push_back(v);
    }

    mesh->vertexBuffer = device->newBuffer(
        vertices.data(),
        vertices.size() * sizeof(Vertex),
        MTL::ResourceStorageModeShared
    );

    mesh->indexBuffer = device->newBuffer(
        rawMesh.indices.data(),
        rawMesh.indices.size() * sizeof(uint32_t),
        MTL::ResourceStorageModeShared
    );

    mesh->vertexCount = static_cast<uint32_t>(rawMesh.vertices.size());
    mesh->indexCount  = static_cast<uint32_t>(rawMesh.indices.size());

    return mesh;
}
//-------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> finalizePNTC(
    MTL::Device* device,
    const RawMesh& rawMesh,
    const std::vector<simd::float3>& normals,
    const std::vector<simd::float2>& uvs,
    const std::vector<simd::float3>& color) 
{
    auto mesh = std::make_unique<Mesh>();
    using Vertex = Rendering::PNTCVertex;
    std::vector<Vertex> vertices;
    vertices.reserve(rawMesh.vertices.size());

    // Build GPU vertex buffer (position + normal)
    for (size_t i = 0; i < rawMesh.vertices.size(); ++i)
    {
        Vertex v;
        v.position = rawMesh.vertices[i];
        v.normal   = normals[i];
        v.texture  = uvs[i];
        v.color    = color[i];

        vertices.push_back(v);
    }

    mesh->vertexBuffer = device->newBuffer(
        vertices.data(),
        vertices.size() * sizeof(Vertex),
        MTL::ResourceStorageModeShared
    );

    mesh->indexBuffer = device->newBuffer(
        rawMesh.indices.data(),
        rawMesh.indices.size() * sizeof(uint32_t),
        MTL::ResourceStorageModeShared
    );

    mesh->vertexCount = static_cast<uint32_t>(rawMesh.vertices.size());
    mesh->indexCount  = static_cast<uint32_t>(rawMesh.indices.size());

    return mesh;
}
//-------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> finalizeMesh(
    MTL::Device* device, 
    const RawMesh& rawMesh, 
    const VertexAttributes& attributes, 
    Rendering::VertexFormat format)
{
    switch (format)
    {
        case Rendering::VertexFormat::PNVertex:
            return finalizePN(device, rawMesh, attributes.normals);

        case Rendering::VertexFormat::PNTVertex:
            return finalizePNT(device, rawMesh, attributes.normals, attributes.uvs);
        
        case Rendering::VertexFormat::PNTCVertex:
            return finalizePNTC(device, rawMesh, attributes.normals, attributes.uvs, attributes.colors);

        default: return nullptr;
    }
}
//----------------------------------------------------------------------------------------------------------------------
} // end namespace
//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> MeshFactory::CreateSimpleCubeMesh(MTL::Device* device, Rendering::VertexFormat format) {
    RawMesh baseMesh = createCubeRaw();

    VertexAttributes attrs;
    attrs.normals = calculateNormals(baseMesh);
    return finalizeMesh(device, baseMesh, attrs, format);
}
//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> MeshFactory::Create24FacesCube(MTL::Device* device, Rendering::VertexFormat format)
{
    RawMesh baseMesh = createCubeRaw();

    VertexAttributes attrs;
    attrs.normals = buildFlatShadedMeshAndNormals(baseMesh);
    if (Rendering::VertexFormat::PNTVertex == format) attrs.uvs = buildUVs(baseMesh, &calculateCubeUVs);
    return finalizeMesh(device, baseMesh, attrs, format);
}
//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Mesh> MeshFactory::CreateIcoSphereMesh(MTL::Device* device, int level, Rendering::VertexFormat format, bool canonical)
{
    VertexAttributes attrs;
    RawMesh baseMesh;
    Icosphere sphere(level, canonical);

    baseMesh.indices = sphere.generateTriangles(canonical);
    baseMesh.vertices = sphere.generatePoints();

    attrs.normals = baseMesh.vertices;
    attrs.uvs = sphere.generateUVPoints();
    attrs.colors = std::vector<float3>(baseMesh.vertices.size());

    return finalizeMesh(device, baseMesh, attrs, format);
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
