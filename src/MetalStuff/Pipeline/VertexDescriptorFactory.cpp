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
        vd->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerInstance);
    }
    //-------------------------------------------------------------------------------------------------------------------

    void setVertexAttributes(MTL::VertexDescriptor* vd)
    {
        auto attributes = vd->attributes();

        auto pos = attributes->object(0);
        pos->setFormat(MTL::VertexFormatFloat3);
        pos->setBufferIndex(0);
        pos->setOffset(offsetof(Rendering::RenderVertex, pos));

        auto vel = attributes->object(1);
        vel->setFormat(MTL::VertexFormatFloat3);
        vel->setBufferIndex(0);
        vel->setOffset(offsetof(Rendering::RenderVertex, vel));

        auto acc = attributes->object(2);
        acc->setFormat(MTL::VertexFormatFloat3);
        acc->setBufferIndex(0);
        acc->setOffset(offsetof(Rendering::RenderVertex, acc));
    }
    //-------------------------------------------------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------------------------------------------------

VertexDescriptorFactory::VertexDescriptorPtr VertexDescriptorFactory::create(){

    using Vertex = Rendering::RenderVertex;

    auto vd = createBaseDescriptor();
    setVertexAttributes(vd);
    setLayout<Vertex>(vd);
    return VertexDescriptorPtr(vd);
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

