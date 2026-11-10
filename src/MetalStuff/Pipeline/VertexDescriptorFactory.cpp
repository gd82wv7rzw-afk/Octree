#include "VertexDescriptorFactory.h"

#include <MetalKit/MetalKit.hpp>
#include <cstddef>

using namespace MetalStuff::Pipeline;

namespace
{
    MTL::VertexDescriptor* createBaseDescriptor()
    {
        return MTL::VertexDescriptor::alloc()->init();
    }
    //-------------------------------------------------------------------------------------------------------------------

    template <typename Vertex>
    void setLayout(MTL::VertexDescriptor* vd)
    {
        vd->layouts()->object(0)->setStride(sizeof(Vertex));
        vd->layouts()->object(0)->setStepRate(1);
        vd->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
    }
    //-------------------------------------------------------------------------------------------------------------------

    void setSimulationAttributes(MTL::VertexDescriptor* vd)
    {
        auto attributes = vd->attributes();

        auto pos = attributes->object(0);
        pos->setFormat(MTL::VertexFormatFloat3);
        pos->setBufferIndex(0);
        pos->setOffset(offsetof(Rendering::SimulationVertex, pos));

        auto vel = attributes->object(1);
        vel->setFormat(MTL::VertexFormatFloat3);
        vel->setBufferIndex(0);
        vel->setOffset(offsetof(Rendering::SimulationVertex, vel));

        auto acc = attributes->object(2);
        acc->setFormat(MTL::VertexFormatFloat3);
        acc->setBufferIndex(0);
        acc->setOffset(offsetof(Rendering::SimulationVertex, acc));

        auto ePot = attributes->object(3);
        ePot->setFormat(MTL::VertexFormatFloat);
        ePot->setBufferIndex(0);
        ePot->setOffset(offsetof(Rendering::SimulationVertex, ePot));
    }
    //-------------------------------------------------------------------------------------------------------------------

    void setPNAttributes(MTL::VertexDescriptor* vd)
    {
        auto attributes = vd->attributes();

        auto pos = attributes->object(0);
        pos->setFormat(MTL::VertexFormatFloat3);
        pos->setBufferIndex(0);
        pos->setOffset(offsetof(Rendering::PNVertex, pos));

        auto norm = attributes->object(1);
        norm->setFormat(MTL::VertexFormatFloat3);
        norm->setBufferIndex(0);
        norm->setOffset(offsetof(Rendering::PNVertex, normale));
    }
    //-------------------------------------------------------------------------------------------------------------------

    void setPNTAttributes(MTL::VertexDescriptor* vd)
    {
        auto attributes = vd->attributes();

        auto pos = attributes->object(0);
        pos->setFormat(MTL::VertexFormatFloat3);
        pos->setBufferIndex(0);
        pos->setOffset(offsetof(Rendering::PNTVertex, position));

        auto norm = attributes->object(1);
        norm->setFormat(MTL::VertexFormatFloat3);
        norm->setBufferIndex(0);
        norm->setOffset(offsetof(Rendering::PNTVertex, normal));

        auto tex = attributes->object(2);
        tex->setFormat(MTL::VertexFormatFloat2);
        tex->setBufferIndex(0);
        tex->setOffset(offsetof(Rendering::PNTVertex, texture));
    }
    //-------------------------------------------------------------------------------------------------------------------

    void setPNTCAttributes(MTL::VertexDescriptor* vd)
    {
        auto attributes = vd->attributes();

        auto pos = attributes->object(0);
        pos->setFormat(MTL::VertexFormatFloat3);
        pos->setBufferIndex(0);
        pos->setOffset(offsetof(Rendering::PNTCVertex, position));

        auto norm = attributes->object(1);
        norm->setFormat(MTL::VertexFormatFloat3);
        norm->setBufferIndex(0);
        norm->setOffset(offsetof(Rendering::PNTCVertex, normal));

        auto tex = attributes->object(2);
        tex->setFormat(MTL::VertexFormatFloat2);
        tex->setBufferIndex(0);
        tex->setOffset(offsetof(Rendering::PNTCVertex, texture));

        auto color = attributes->object(3);
        color->setFormat(MTL::VertexFormatFloat3);
        color->setBufferIndex(0);
        color->setOffset(offsetof(Rendering::PNTCVertex, color));
    }
    //-------------------------------------------------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------------------------------------------------

VertexDescriptorFactory::VertexDescriptorPtr VertexDescriptorFactory::create(Rendering::VertexFormat format)
{
    using namespace Rendering;
    switch (format)
    {
        case VertexFormat::PNVertex:
        {
            using Vertex = Rendering::PNVertex;

            auto vd = createBaseDescriptor();
            setPNAttributes(vd);
            setLayout<Vertex>(vd);

            return VertexDescriptorPtr(vd);
        }

        case VertexFormat::PNTVertex:
        {
            using Vertex = Rendering::PNTVertex;

            auto vd = createBaseDescriptor();
            setPNTAttributes(vd);
            setLayout<Vertex>(vd);

            return VertexDescriptorPtr(vd);
        }

        case VertexFormat::PNTCVertex:
        {
            using Vertex = Rendering::PNTCVertex;

            auto vd = createBaseDescriptor();
            setPNTCAttributes(vd);
            setLayout<Vertex>(vd);

            return VertexDescriptorPtr(vd);
        }

        case VertexFormat::SimulationVertex:
        {
            using Vertex = Rendering::SimulationVertex;

            auto vd = createBaseDescriptor();
            setSimulationAttributes(vd);
            setLayout<Vertex>(vd);

            return VertexDescriptorPtr(vd);
        }

        default: return nullptr;
    }
}
//----------------------------------------------------------------------------------------------------------------------

void VertexDescriptorFactory::VertexDescriptorDeleter::operator()(MTL::VertexDescriptor* ptr) const
{
    if (ptr)
    {
        ptr->release();
    }
}
//----------------------------------------------------------------------------------------------------------------------

