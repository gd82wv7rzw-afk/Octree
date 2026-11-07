#pragma once

#include "Rendering/Vertices.h"
#include <memory>

namespace MTL { class VertexDescriptor; }

namespace MetalStuff::Pipeline {

/// @brief Creates Metal vertex descriptors from engine vertex formats.
/// @details
/// Maps VertexFormat to a configured MTL::VertexDescriptor for Metal pipelines.
/// Uses RAII (unique_ptr with custom deleter) for proper Metal release handling.
class VertexDescriptorFactory
{
private:
    /// @brief Custom deleter for Metal VertexDescriptor objects.
    /// @details
    /// Metal objects use manual reference counting (alloc/init/release).
    /// This ensures proper release when the unique_ptr goes out of scope.
    struct VertexDescriptorDeleter
    {
        /// @brief Releases the Metal vertex descriptor if valid.
        /// @param ptr Pointer to Metal vertex descriptor.
        void operator()(MTL::VertexDescriptor* ptr) const;
    };

public:
    using VertexDescriptorPtr = std::unique_ptr<MTL::VertexDescriptor, VertexDescriptorDeleter>;

    /// @brief Creates a Metal vertex descriptor for the given format.
    /// @param format Engine vertex format.
    /// @return Owning RAII pointer to a configured descriptor.
    static VertexDescriptorPtr create(Rendering::VertexFormat format);
};
//----------------------------------------------------------------------------------------------------------------------

} // namespace MetalStuff::Pipeline ------------------------------------------------------------------------------------