#pragma once

namespace Gui {

struct FileNames
{
    std::string defaultRootFolder{"~/Desktop"};
    std::string defaultProjectName{"New Simulation"};
    std::string outputFolder{defaultRootFolder + "/" + defaultProjectName};
    std::string trajectoryFolder{defaultRootFolder + "/trj"};
};
//----------------------------------------------------------------------------------------------------------------------

struct GuiParameter
{
    int nAtoms{10};

    float freeLength{1e2};

    FileNames fileNames;

    int trajectoryFrequency{100};

    bool recordTrajectory{false};

    bool randomConfiguration{true};

};
//----------------------------------------------------------------------------------------------------------------------

} 
// namespace Gui -------------------------------------------------------------------------------------------------------