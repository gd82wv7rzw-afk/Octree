#pragma once

namespace Gui {

struct FileNames
{
    std::string defaultRootFolder{"~/Desktop"};
    std::string defaultProjectName{"New Simulation"};
    std::string outputFolder{defaultRootFolder + "/" + defaultProjectName};
    std::string trajectoryFolder{defaultRootFolder + "/trj"};
    std::string startConfig{outputFolder + "/input.xyz"};
};
//----------------------------------------------------------------------------------------------------------------------

struct SimulationSettings
{
    int nAtoms{int(1e5)};
    float meanFreeDistance{1e2};
    bool randomConfiguration{true};
    std::atomic<float> temperature{1.f};
    std::atomic<float> dt{0.1f};
};
//----------------------------------------------------------------------------------------------------------------------

struct RenderingSettings
{
    float scale{1.f};
};

struct GuiParameter
{
    FileNames fileNames;
    SimulationSettings simulationSettings;
    RenderingSettings renderSettings;

    int trajectoryFrequency{100};
    bool recordTrajectory{false};
};
//----------------------------------------------------------------------------------------------------------------------

} 
// namespace Gui -------------------------------------------------------------------------------------------------------