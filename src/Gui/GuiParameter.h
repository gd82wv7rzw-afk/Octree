#pragma once

namespace Gui {

struct GuiParameter
{
    int nAtoms{10};

    float freeLength{1e2};

    std::string outputFolder{"~/Desktop"};

    bool recordTrajectory{false};

    bool randomConfiguration{true};

};
//----------------------------------------------------------------------------------------------------------------------

} 
// namespace Gui -------------------------------------------------------------------------------------------------------