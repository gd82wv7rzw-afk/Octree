#include "Renderer.h"
#include "MetalStuff/Pipeline/VertexDescriptorFactory.h"
#include "Rendering/CameraUniforms.h"

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

    mTestMesh = MeshFactory::CreateIcoSphereMesh(mMetalDevice, 2, VertexFormat::PNVertex, true);

    mCameraBuffer = mMetalDevice->newBuffer(sizeof(CameraUniforms), MTL::ResourceStorageModeShared);
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

    if (mCameraBuffer)
    {
        mCameraBuffer->release();
        mCameraBuffer = nullptr;
    }
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

void Renderer::updateFrame(CA::MetalDrawable* drawable, MTL::RenderPassDescriptor* renderPass, Buffer* buf)
{
    if (!drawable || !renderPass || !buf) return;

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

    encoder->setRenderPipelineState(mPipeline.get());
    encoder->setDepthStencilState(mDepthStencilState);
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeBack);

    //encoder->setVertexBuffer(buf->buffer(), 0, 0);
    encoder->setVertexBuffer(mTestMesh->vertexBuffer, 0, 0);
    encoder->setVertexBuffer(mCameraBuffer, 0, 1);

    encoder->setTriangleFillMode(MTL::TriangleFillModeLines);

    encoder->drawIndexedPrimitives(
        MTL::PrimitiveTypeTriangle,
        mTestMesh->indexCount,
        MTL::IndexTypeUInt32,
        mTestMesh->indexBuffer,
        0
    );
/*
    encoder->drawPrimitives(
        MTL::PrimitiveTypeTriangle,
        NS::UInteger(0),
        NS::UInteger(3),
        NS::UInteger(buf->size())
    );
*/
    encoder->endEncoding();

    commandBuffer->presentDrawable(drawable);
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();

    pool->release();
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::buildPipeline()
{
    assert(mMetalDevice != nullptr);

    PipelineBuilder builder(mMetalDevice);

    mPipeline = builder.buildRender(
        mPixelFormat, 
        MTL::PixelFormatDepth32Float,
        VertexFormat::PNVertex, 
        mSampleCount, 
        "/default.metallib", 
        "vertexMain", 
        "fragmentMain");
}
//----------------------------------------------------------------------------------------------------------------------

void Renderer::updateCamera()
{
    mCamera.setPerspective(
        0.785398f,
        mWidth / mHeight,
        0.1f,
        1000.0f
    );

    auto* uniforms = static_cast<CameraUniforms*>(mCameraBuffer->contents());

    uniforms->view       = mCamera.viewMatrix();
    uniforms->projection = mCamera.projectionMatrix();
}
//----------------------------------------------------------------------------------------------------------------------