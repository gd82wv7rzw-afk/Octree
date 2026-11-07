#pragma once

#include "MetalStuff/Pipeline/PipelineBuilder.h"
#include "MetalStuff/MetalArray.hpp"
#include "Rendering/Vertices.h"
#include "Rendering/Camera.h"
#include "Rendering/Meshes/Mesh.h"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>


namespace Rendering {

class Renderer
{
    public:
 
        using Buffer = MetalArray<SimulationVertex>;

        Renderer( MTL::Device* device, MTL::CommandQueue* commandQueue);

        ~Renderer();

        void setPixelFormat(MTL::PixelFormat pixelFormat) { mPixelFormat = pixelFormat;}

        void resize(float width, float height);

        void updateFrame(CA::MetalDrawable* drawable, MTL::RenderPassDescriptor* renderPass, Buffer* buf);

    private:

        void buildPipeline();
        void buildDepthStencilState();
        void createDepthAndMSAATextures();
        void updateCamera();
        //--------------------------------------------------------------------------------------------------------------

        // metal core
        MTL::Device*        mMetalDevice  = nullptr;
        MTL::CommandQueue*  mCommandQueue = nullptr;
        MTL::PixelFormat    mPixelFormat  = MTL::PixelFormatBGRA8Unorm;
        //--------------------------------------------------------------------------------------------------------------
 
        // Pipeline system
        MetalStuff::Pipeline::RenderPipelinePtr mPipeline;
        //--------------------------------------------------------------------------------------------------------------
 
        // Render state
        //MTL::RenderPassDescriptor* mRenderPass = nullptr;
        MTL::DepthStencilState* mDepthStencilState = nullptr;
        //--------------------------------------------------------------------------------------------------------------
 
        // GPU resources
        MTL::Texture* mMsaaRenderTargetTexture = nullptr;
        MTL::Texture* mDepthTexture = nullptr;
        //--------------------------------------------------------------------------------------------------------------

        uint32_t mSampleCount = 4;
        //--------------------------------------------------------------------------------------------------------------
 
        // Weitere Ressourcen wie bisher
        float mWidth = 1.0f;
        float mHeight = 1.0f;  
        Rendering::Camera mCamera;
        MTL::Buffer* mCameraBuffer{nullptr};

        std::unique_ptr<Mesh> mTestMesh;
        //--------------------------------------------------------------------------------------------------------------     
};
//----------------------------------------------------------------------------------------------------------------------

}// namespace Renderering ----------------------------------------------------------------------------------------------
