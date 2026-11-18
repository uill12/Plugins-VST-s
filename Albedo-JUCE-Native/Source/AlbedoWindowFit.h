#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>

// CP34: display-aware 3:2 editor dimensions, expressed in JUCE logical pixels.
// The Bitwig title bar and Windows taskbar need real headroom; starting at
// 1440x960 previously pushed the WebView2 editor below the physical screen.
namespace albedo_window_fit
{
struct Geometry
{
    int initialWidth=1152;
    int initialHeight=768;
    int minWidth=840;
    int minHeight=560;
    int maxWidth=1416;
    int maxHeight=944;
};

inline Geometry calculate()
{
    Geometry g;
    int availableW=1920, availableH=1040;
    if(const auto* display=juce::Desktop::getInstance().getDisplays().getPrimaryDisplay())
    {
        availableW=display->userArea.getWidth();
        availableH=display->userArea.getHeight();
    }
    // Keep 96 logical pixels laterally and 130 vertically for the host
    // decorations, taskbar and docking on a 15.6-inch laptop.
    const int byW=std::max(480,availableW-96);
    const int byH=std::max(480,(std::max(320,availableH-130)*3)/2);
    const int upper=std::min({1800,byW,byH});
    g.maxWidth=std::max(480,(upper/6)*6);
    g.maxHeight=g.maxWidth*2/3;
    g.minWidth=std::min(840,g.maxWidth);
    g.minHeight=g.minWidth*2/3;
    g.initialWidth=std::min(1152,g.maxWidth);
    g.initialWidth=(g.initialWidth/6)*6;
    g.initialHeight=g.initialWidth*2/3;
    return g;
}
} // namespace albedo_window_fit
