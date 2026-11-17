#pragma once
#include "AlbedoProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include <vector>

class AlbedoEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AlbedoEditor (AlbedoProcessor&);
    ~AlbedoEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct AlbedoLookAndFeel;
    struct Module;
    struct ParamControl;
    struct ArborView;
    struct DynamicGraph;
    struct Atlas;

    AlbedoProcessor& processor;
    std::unique_ptr<AlbedoLookAndFeel> theme;
    std::unique_ptr<juce::Drawable> logo;
    juce::TextButton atlasButton {"PRESET ATLAS"}, allControlsButton {"ALL CONTROLS"};
    juce::TextButton closeAdvanced {"CLOSE"};
    juce::ComboBox sourceSelector;
    std::unique_ptr<Atlas> atlas;
    std::unique_ptr<ArborView> arbor;
    juce::OwnedArray<DynamicGraph> graphs;
    DynamicGraph* enginePlots[4] {};
    DynamicGraph* filterPlots[2] {};
    DynamicGraph* inspectorPlot = nullptr;
    DynamicGraph* envelopePlot = nullptr;
    DynamicGraph* cellularPlot = nullptr;
    DynamicGraph* effectsPlot = nullptr;
    DynamicGraph* pulsePlot = nullptr;
    int animationTick = 0;
    juce::Component advancedPanel;
    juce::Viewport advancedViewport;
    juce::Component advancedContent;
    std::unique_ptr<juce::ResizableCornerComponent> resizeHandle;
    juce::ComponentBoundsConstrainer constrainer;
    juce::OwnedArray<Module> modules;
    juce::OwnedArray<ParamControl> controls;
    juce::OwnedArray<ParamControl> advancedOwned;
    juce::Label advancedBackground;
    std::vector<ParamControl*> inspectorControls, sourceRackControls, macroControls, filterControls,
                               envelopeControls, couplingControls, cellularControls, fxControls, routingControls,
                               stepControls, advancedControls;
    juce::OwnedArray<juce::TextButton> engineButtons;
    std::vector<juce::Rectangle<int>> moduleRects;
    int activeSource=0;
    bool showAdvanced=false;
    void timerCallback() override;
    ParamControl* makeControl (uint32_t id, bool dial=true, bool label=true);
    void setupModules();
    void layoutControls (const std::vector<ParamControl*>&, juce::Rectangle<int>, int columns);
    void drawSection (juce::Graphics&, juce::Rectangle<float>, const juce::String&, int index);
    void setAdvancedVisible (bool visible);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlbedoEditor)
};
