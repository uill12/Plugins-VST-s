#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vitriol_dsp.hpp>
#include <albedo_factory_patches.hpp>
#include "AlbedoCreativeBank.hpp"
#include "AlbedoRearField.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

class AlbedoProcessor final : public juce::AudioProcessor, private juce::AudioProcessorParameter::Listener
{
public:
    AlbedoProcessor();
    ~AlbedoProcessor() override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock (juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Albedo Quintessential Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 18.0; }
    int getNumPrograms() override { return 64 + (int) albedo_creative::patches.size() + (int) albedo_morph::patches.size(); }
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioParameterFloat* parameter (uint32_t id) const noexcept;
    const std::vector<vitriol::ParamDef>& definitions() const noexcept { return definitions_; }
    void loadFactoryPreset (int index);
    float paramValue (uint32_t id) const noexcept;
    int arpStep() const noexcept {return synth->arpStep();}
    int activeVoices() const noexcept { return activeVoiceCount.load(); }

private:
    std::unique_ptr<vitriol::Synth> synth;
    std::vector<vitriol::ParamDef> definitions_;
    std::unordered_map<uint32_t,juce::AudioParameterFloat*> idParameters;
    std::vector<std::pair<uint32_t,juce::AudioParameterFloat*>> parameterList;
    std::vector<double> lastValues;
    std::atomic<int> currentProgram { 0 };
    std::atomic<int> activeVoiceCount { 0 };
    bool transportPlaying=false;
    AlbedoRearField rearField;
    std::atomic<bool> parametersChanged { true };
    void parameterValueChanged(int, float) override { parametersChanged.store(true, std::memory_order_release); }
    void parameterGestureChanged(int, bool) override {}
    template<typename T> void renderRear(juce::AudioBuffer<T>& b)
    {
        if (b.getNumChannels()==4)
            rearField.render(b.getReadPointer(0), b.getReadPointer(1), b.getWritePointer(2),
                             b.getWritePointer(3), b.getNumSamples());
    }
    void syncArpClock();
    void updateSynthParameters();
    void handleMidi (const juce::MidiMessage&);
    void processRegion (juce::AudioBuffer<float>& buffer, int start, int length);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlbedoProcessor)
};
