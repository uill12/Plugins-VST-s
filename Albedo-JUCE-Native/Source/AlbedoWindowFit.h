#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>

// CP39 — the live HTML design canvas is 1500x1000 (3:2).
// Start at a comfortable 1200x800 viewport, subject to available screen space.
// Preserve the Bitwig chrome/taskbar clearance and the exact 3:2 ratio.
namespace albedo_window_fit
{
struct Geometry
{
    int initialWidth=1200;
    int initialHeight=800;
    int minWidth=720;
    int minHeight=480;
    int maxWidth=1458;
    int maxHeight=972;
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
    const int byW=std::max(480,availableW-80);
    const int byH=std::max(320,availableH-95);
    // Restrict 3:2 editor dimensions together, never independently.
    const int allowed=std::min({1500,byW,(byH*3)/2});
    g.maxWidth=std::max(480,(allowed/6)*6);
    g.maxHeight=g.maxWidth*2/3;
    g.minWidth=std::min(720,g.maxWidth);
    g.minHeight=g.minWidth*2/3;
    g.initialWidth=std::min(1200,g.maxWidth);
    g.initialWidth=(g.initialWidth/6)*6;
    g.initialHeight=g.initialWidth*2/3;
    return g;
}
} // namespace albedo_window_fit
