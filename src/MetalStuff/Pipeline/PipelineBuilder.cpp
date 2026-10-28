#include "MetalStuff/Pipeline/PipelineBuilder.h"

#include <cstddef>
#include <iostream>

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

using namespace MetalStuff::Pipeline;

PipelineBuilder::PipelineBuilder(MTL::Device* device) : mDevice(device)
{}
//----------------------------------------------------------------------------------------------------------------------

ComputePipelinePtr PipelineBuilder::buildCompute(const char* filepath, const char* kernelName, const char* rootDir) {
    NS::Error* error = nullptr;

    // load compiled library
    auto* lib = readLibrary(filepath, mDevice, rootDir);

    // load compute kernel
    auto* kernelFunction = lib->newFunction( NS::String::string(kernelName, NS::UTF8StringEncoding) );

    if (!kernelFunction) {
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

