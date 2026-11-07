#include "MetalStuff/Pipeline/PipelineBuilder.h"
#include "MetalStuff/Pipeline/VertexDescriptorFactory.h"

#include <cstddef>
#include <iostream>

using namespace MetalStuff::Pipeline;

namespace {
    MTL::Library* readLibrary(const char* filepath, MTL::Device* device, const char* rootDir)
    {
        std::string base = rootDir ? rootDir : NS::Bundle::mainBundle()->resourcePath()->utf8String();
        std::string path = base + filepath;
        NS::Error* error = nullptr;
 
        // Datei als URL laden (wichtig!)
        NS::URL* url = NS::URL::fileURLWithPath(
            NS::String::string(path.c_str(), NS::UTF8StringEncoding)
        );
 
        MTL::Library* library = device->newLibrary(url, &error);
 
        if (!library)
        {
            std::cerr << "Failed to load metallib.\n";
            std::cerr << error->localizedDescription()->utf8String() << std::endl;
            std::exit(-1);
        }
 
        return library;
    }
//----------------------------------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------------------------

PipelineBuilder::PipelineBuilder(MTL::Device* device) : mDevice(device)
{}
//----------------------------------------------------------------------------------------------------------------------

ComputePipelinePtr PipelineBuilder::buildCompute(const char* filepath, const char* kernelName, const char* rootDir) {
    NS::Error* error = nullptr;

    // load compiled library
    auto* lib = readLibrary(filepath, mDevice, rootDir);

    // load compute kernel
    auto* kernelFunction = lib->newFunction( NS::String::string(kernelName, NS::UTF8StringEncoding) );

    if (!kernelFunction) 
    {
        std::cerr << "PipelineBuilder: Failed to load compute kernel\n";
        lib->release();
        std::exit(-1);
    }

    // create compute pipeline state
    auto* rawPipeline = mDevice->newComputePipelineState( kernelFunction, &error );

    if (!rawPipeline)
    {
        std::cerr << "PipelineBuilder: Failed to create ComputePipelineState\n";

        if (error) std::cerr << error->localizedDescription()->utf8String() << "\n";

        kernelFunction->release();
        lib->release();

        std::exit(-1);
    }

    // cleanup
    kernelFunction->release();
    lib->release();

    return ComputePipelinePtr(rawPipeline);
}
//----------------------------------------------------------------------------------------------------------------------

RenderPipelinePtr PipelineBuilder::buildRender(
    MTL::PixelFormat pixelformat, 
    MTL::PixelFormat depthPixelformat, 
    Rendering::VertexFormat format, 
    uint32_t sampleCount,
    const char* filepath,
    const char* vertexName,
    const char* fragmentName,
    const char* rootDir)
{
    NS::Error* error = nullptr;
    auto vertexDescriptor = VertexDescriptorFactory::create(format);

    // load the compiled library
    auto lib = readLibrary(filepath, mDevice, rootDir);

    // Shader functions (from injected library)
    auto* vertexFunction = lib->newFunction(NS::String::string(vertexName, NS::UTF8StringEncoding));

    auto* fragmentFunction = lib->newFunction(NS::String::string(fragmentName, NS::UTF8StringEncoding));

    if (!vertexFunction || !fragmentFunction)
    {
        std::cerr << "PipelineBuilder: Failed to load shader functions\n";

        lib->release();
        std::exit(-1);
    }

    // Pipeline descriptor setup
    auto* desc = MTL::RenderPipelineDescriptor::alloc()->init();

    desc->setLabel(NS::String::string("RenderPipeline", NS::UTF8StringEncoding));

    desc->setVertexFunction(vertexFunction);
    desc->setFragmentFunction(fragmentFunction);

    desc->setVertexDescriptor(vertexDescriptor.get());
    desc->setSampleCount(sampleCount);

    // Color attachment (from CAMetalLayer)
    desc->colorAttachments()->object(0)->setPixelFormat(pixelformat);

    // Depth format
    desc->setDepthAttachmentPixelFormat(depthPixelformat);

    // Create pipeline state
    MTL::RenderPipelineState* rawPipeline = mDevice->newRenderPipelineState(desc, &error);

    if (!rawPipeline)
    {
        std::cerr << "PipelineBuilder: Failed to create RenderPipelineState\n";

        if (error)
        {
            std::cerr << error->localizedDescription()->utf8String() << "\n";
        }

        desc->release();
        vertexFunction->release();
        fragmentFunction->release();
        lib->release();

        std::exit(-1);
    }

    // Cleanup temporary Metal objects
    desc->release();
    vertexFunction->release();
    fragmentFunction->release();
    lib->release();

    return MetalStuff::Pipeline::RenderPipelinePtr(rawPipeline);
}
//----------------------------------------------------------------------------------------------------------------------