#include "Renderer.h"
#include "MetalStuff/Pipeline/VertexDescriptorFactory.h"
#include "Simulation/PostProzess.h"

#include <stdexcept>
#include <Foundation/Foundation.hpp>

using namespace Rendering;

Renderer::Renderer(MTL::Device* device, MTL::CommandQueue* commandQueue)
    : mMetalDevice(device)
    , mCommandQueue(commandQueue)
{
    if (!mMetalDevice || !mCommandQueue) throw std::invalid_argument("Renderer: Invalid Metal device or command queue");

    buildPipeline();
    buildDepthStencilState();
    createDepthAndMSAATextures();

    //mTestMesh = MeshFactory::CreateIcoSphereMesh(mMetalDevice, 4, VertexFormat::PNVertex, true);
    //Create24FacesCube(mMetalDevice, VertexFormat::PNVertex);
    rebuildSphere(4);

    updateCamera();
}
//----------------------------------------------------------------------------------------------------------------------

Renderer::~Renderer()
{
    // GPU resources (Metal retain/release ownership)
    if (mMsaaRenderTargetTexture) 
    {
        mMsaaRenderTargetTexture->release();
        mMsaaRenderTargetTexture = nullptr;
    }

    if (mDepthTexture) 
    {
        mDepthTexture->release();
        mDepthTexture = nullptr;
    }

    if (mDepthStencilState) 
    {
        mDepthStencilState->release();
        mDepthStencilState = nullptr;
    }
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::buildPipeline()
{
    assert(mMetalDevice != nullptr);

    PipelineBuilder builder(mMetalDevice);

    mPipeline = builder.buildRender
    (
        mPixelFormat, 
        MTL::PixelFormatDepth32Float,
        VertexFormat::PNVertex, 
        mSampleCount, 
        "/default.metallib", 
        "vertexMain", 
        "fragmentMain"
    );
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::rebuildSphere(int resolution)
{
    mTestMesh.release();
    mTestMesh = (MeshFactory::CreateIcoSphereMesh(mMetalDevice, resolution, VertexFormat::PNVertex, true));
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::buildDepthStencilState() 
{
    MTL::DepthStencilDescriptor* depthInfo = MTL::DepthStencilDescriptor::alloc()->init();
    depthInfo->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLessEqual);
    depthInfo->setDepthWriteEnabled(true);
    mDepthStencilState = mMetalDevice->newDepthStencilState(depthInfo);   
    depthInfo->release();
}
//---------------------------------------------------------------------------------------------------------------------

void Renderer::createDepthAndMSAATextures()
{
    // MSAA color render target
    MTL::TextureDescriptor* msaaColorDesc = MTL::TextureDescriptor::alloc()->init();

    msaaColorDesc->setTextureType(MTL::TextureType2DMultisample);
    msaaColorDesc->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    msaaColorDesc->setWidth(mWidth);
    msaaColorDesc->setHeight(mHeight);
    msaaColorDesc->setSampleCount(mSampleCount);
    msaaColorDesc->setUsage(MTL::TextureUsageRenderTarget);
    msaaColorDesc->setStorageMode(MTL::StorageModePrivate);

    mMsaaRenderTargetTexture = mMetalDevice->newTexture(msaaColorDesc);

    // MSAA depth texture
    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::alloc()->init();

    depthDesc->setTextureType(MTL::TextureType2DMultisample);
    depthDesc->setPixelFormat(MTL::PixelFormatDepth32Float);
    depthDesc->setWidth(mWidth);
    depthDesc->setHeight(mHeight);
    depthDesc->setSampleCount(mSampleCount);
    depthDesc->setUsage(MTL::TextureUsageRenderTarget);
    depthDesc->setStorageMode(MTL::StorageModePrivate);

    mDepthTexture = mMetalDevice->newTexture(depthDesc);

    // -------------------------------------------------------------------------
    // cleanup descriptors (CPU side only)
    // -------------------------------------------------------------------------
    msaaColorDesc->release();
    depthDesc->release();
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::resize(float width, float height)
{
    if (width <= 0.0f || height <= 0.0f) return;

    mWidth = width;
    mHeight = height;

    // Release size-dependent GPU resources
    if (mMsaaRenderTargetTexture) 
    {
        mMsaaRenderTargetTexture->release();
        mMsaaRenderTargetTexture = nullptr;
    }

    if (mDepthTexture) 
    {
        mDepthTexture->release();
        mDepthTexture = nullptr;
    }

    // Recreate GPU resources with new dimensions
    createDepthAndMSAATextures();

    updateCamera();
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::updateFrame
(
    CA::MetalDrawable* drawable, 
    MTL::RenderPassDescriptor* renderPass, 
    Buffer* buf,
    float scale
)
{
    if (!drawable || !renderPass || !buf) return;

    scale *= mBaseScale;

    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer* commandBuffer = mCommandQueue->commandBuffer();

    if (!commandBuffer)
    {
        pool->release();
        return;
    } 

    // Render pass setup
    auto color = renderPass->colorAttachments()->object(0);
    color->setTexture(mMsaaRenderTargetTexture);
    color->setResolveTexture(drawable->texture());
    color->setLoadAction(MTL::LoadActionClear);
    color->setClearColor(MTL::ClearColor(0.0, 0.0, 0.0, 1.0));
    color->setStoreAction(MTL::StoreActionMultisampleResolve);

    auto depth = renderPass->depthAttachment();
    depth->setTexture(mDepthTexture);
    depth->setLoadAction(MTL::LoadActionClear);
    depth->setStoreAction(MTL::StoreActionDontCare);
    depth->setClearDepth(1.0);

    MTL::RenderCommandEncoder* encoder = commandBuffer->renderCommandEncoder(renderPass);

    if (!encoder)
    {
        pool->release();
        return;
    }

    Simulation::PostProzess postProcess;
    auto eKin = postProcess.eKin(buf);

    encoder->setRenderPipelineState(mPipeline.get());
    encoder->setDepthStencilState(mDepthStencilState);
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeBack);

    encoder->setVertexBuffer(mTestMesh->vertexBuffer, 0, 0);
    encoder->setVertexBuffer(buf->buffer(), 0, 1);
    encoder->setVertexBytes(&mTransformController.pvmMatrix(), sizeof(simd::float4x4), 2);
    encoder->setVertexBytes(&mTransformController.rotMatrix(), sizeof(simd::float4x4), 3);
    encoder->setVertexBytes(&scale, sizeof(float), 4);
    encoder->setVertexBytes(&eKin, sizeof(simd::float3), 5);

    //encoder->setTriangleFillMode(MTL::TriangleFillModeLines);

    encoder->drawIndexedPrimitives
    (
        MTL::PrimitiveTypeTriangle,
        mTestMesh->indexCount,
        MTL::IndexTypeUInt32,
        mTestMesh->indexBuffer,
        0,
        buf->size()
    );

    encoder->endEncoding();

    commandBuffer->presentDrawable(drawable);
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    pool->release();
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::updateCamera()
{
    mTransformController.setPerspective
    (
        M_PI_4,
        mWidth / mHeight,
        0.1f,
        100.0f
    );
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::mouseDragged(float dx, float dy, bool controlPressed)
{
    if (controlPressed)
    {
        mTransformController.rotZ(dx * 0.05f / M_PI);
        mTransformController.zoom(-dy * 0.02f);
    }
    else
    {
        mTransformController.rotY(dx * 0.02f / M_PI);
        mTransformController.rotX(dy * 0.02f / M_PI);
    }
}
//----------------------------------------------------------------------------------------------------------------------