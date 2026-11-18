#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "AlbedoProcessor.h"
#include "AlbedoEditor.h"
#include <memory>
#include <vector>

class AlbedoWebViewEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    static bool isAvailable();
    explicit AlbedoWebViewEditor(AlbedoProcessor&);
    ~AlbedoWebViewEditor() override;

    void resized() override;
    void paint(juce::Graphics&) override;

private:
    struct Browser final : juce::WebBrowserComponent
    {
        AlbedoWebViewEditor& owner;
        Browser(AlbedoWebViewEditor&, const Options&);
        bool pageAboutToLoad(const juce::String&) override;
        void pageFinishedLoading(const juce::String&) override;
        bool pageLoadHadNetworkError(const juce::String&) override;
    };
    static juce::WebBrowserComponent::Options makeOptions(AlbedoWebViewEditor*);
    std::optional<juce::WebBrowserComponent::Resource> resource(const juce::String&) const;
    void receive(const juce::var&);
    void handleAction(const juce::String&);
    void postActionStatus(const juce::String&);
    void timerCallback() override;
    void activateFallback(const juce::String& reason);
    void sendDelta(bool full);
    void onBrowserPageLoaded();

    AlbedoProcessor& processor;
    Browser browser;
    std::unique_ptr<AlbedoEditor> nativeFallback;
    std::unique_ptr<juce::FileChooser> patchChooser;
    bool patchDialogOpen=false;
    std::vector<float> lastValues;
    bool pageReady=false, browserHasLoaded=false, fullSyncPending=true;
    int pendingTicks=0;
    int lastProgram=-1;
    uint32_t currentGesture=UINT32_MAX;
    juce::String fallbackMessage;
    juce::ComponentBoundsConstrainer aspect;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlbedoWebViewEditor)
};
