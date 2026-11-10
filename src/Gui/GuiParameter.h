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
    int nAtoms{int(1e4)};
    float meanFreeDistance{1e1};
    bool randomConfiguration{true};
    std::atomic<float> temperature{0.998f};
    std::atomic<float> dt{0.005f};
};
//----------------------------------------------------------------------------------------------------------------------

struct RenderingSettings
{
    float scale{1.f};
    int resolution{1};
};
//----------------------------------------------------------------------------------------------------------------------

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