#pragma once

#include "Rendering/Vertices.h"
#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief   forward declarations
namespace MTL {
    class Device;
    class Buffer;
}
//----------------------------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief Base class for all GPU meshes (Quad, Cube, OBJ, etc.)
/// 
/// Design intention:
/// - Holds GPU buffer handles (NOT owns CPU mesh generation)
/// - Does NOT assume valid data at construction time
/// - the user responsible for filling buffers
/// - When buffers where destroied this handle this struct
struct Mesh {
    /// @brief GPU vertex buffer (nullptr until initialized by derived class)
    MTL::Buffer* vertexBuffer = nullptr;

    /// @brief GPU index buffer (nullptr until initialized)
    /// NOTE:
    /// - Optional for non-indexed meshes
    MTL::Buffer* indexBuffer = nullptr;

    /// @brief the number of vertices
    unsigned int vertexCount = 0;

    /// @brief the number of indices
    unsigned int indexCount = 0;

    /// @brief destroi the internal buffers
    ~Mesh();
};
//----------------------------------------------------------------------------------------------------------------------

/// @brief here we build the meshes
namespace MeshFactory {
    /// @brief build a qube mesh with 8 vertices
    /// @param device the metal device
    /// @return std::unique_ptr<Mesh> the mesh
    std::unique_ptr<Mesh> CreateSimpleCubeMesh(MTL::Device* device, Rendering::VertexFormat format);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief build a 24 faces cube mesh
    /// @param device the metal device
    /// @return std::unique_ptr<Mesh> the mesh
    std::unique_ptr<Mesh> Create24FacesCube(MTL::Device *device, Rendering::VertexFormat format);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief build a 24 faces cube mesh
    /// @param device the metal device
    /// @param path where we finde the obj file
    /// @return std::unique_ptr<Mesh> the mesh
    //std::unique_ptr<Mesh> CreateObjMesh(MTL::Device* device, const char* path, Rendering::VertexFormat format);
    //------------------------------------------------------------------------------------------------------------------

    /// @brief Creates a sphere mesh with given subdivision level
    /// @param device the metal device
    /// @param level we start from a icosaeder (level 0) and subdivide every face in for pro level
    /// @return std::unique_ptr<Mesh> the mesh
    std::unique_ptr<Mesh> CreateIcoSphereMesh(MTL::Device* device, int level, Rendering::VertexFormat format, bool canonical=false);    
};
//----------------------------------------------------------------------------------------------------------------------


