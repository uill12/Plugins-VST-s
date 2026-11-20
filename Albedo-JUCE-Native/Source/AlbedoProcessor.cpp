#include "AlbedoProcessor.h"
#include "AlbedoEditor.h"
#include "AlbedoWebViewEditor.h"
#include <cmath>
#include "AlbedoPrograms.hpp"

AlbedoProcessor::AlbedoProcessor()
  : juce::AudioProcessor (juce::AudioProcessor::BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
    synth (std::make_unique<vitriol::Synth>()), definitions_ (synth->descriptors())
{
    lastValues.reserve (definitions_.size());
    parameterList.reserve (definitions_.size());
    for (const auto& d : definitions_)
    {
        const auto range = juce::NormalisableRange<float> ((float) d.min, (float) d.max, d.stepped ? 1.0f : 0.0f);
        auto* p = new juce::AudioParameterFloat (juce::ParameterID ("p" + juce::String (d.id), 1),
                        juce::String (d.name), range, (float) d.def, juce::String (d.unit));
        addParameter (p);
        p->addListener(this);
        idParameters.emplace (d.id, p);
        parameterList.emplace_back (d.id, p);
        lastValues.push_back (d.def);
    }
    loadFactoryPreset(0);
}
AlbedoProcessor::~AlbedoProcessor()
{
    for (const auto& entry : parameterList) entry.second->removeListener(this);
}
void AlbedoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth->prepare (sampleRate);
    juce::ignoreUnused(samplesPerBlock);
    rearField.prepare(sampleRate);
    parametersChanged.store(true, std::memory_order_release);
    updateSynthParameters();
}
void AlbedoProcessor::releaseResources() { synth->allNotesOff(); }
bool AlbedoProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{ return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() || l.getMainOutputChannelSet() == juce::AudioChannelSet::quadraphonic(); }
void AlbedoProcessor::updateSynthParameters()
{
    if (auto* transport=getPlayHead()) {
        if(auto position=transport->getPosition()) {
            if(auto bpm=position->getBpm()) synth->setArpTempo(*bpm);
            const bool playing=position->getIsPlaying();
            if(transportPlaying && !playing && synth->getParam(6020)>.5) synth->allNotesOff();
            transportPlaying=playing;
        }
    }
    if (!parametersChanged.exchange(false, std::memory_order_acq_rel)) return;
    // Audio-thread-owned lastValues; no locks, GUI operations, or allocations.
    for (size_t i = 0; i < parameterList.size(); ++i)
    {
        const auto val = (double) parameterList[i].second->get();
        if (val != lastValues[i]) { synth->setParam (parameterList[i].first, val); lastValues[i] = val; }
    }
}
void AlbedoProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn()) synth->noteOn (m.getNoteNumber(), m.getFloatVelocity(), m.getChannel() - 1);
    else if (m.isNoteOff()) synth->noteOff (m.getNoteNumber(), m.getChannel() - 1);
    else if (m.isAllNotesOff() || m.isAllSoundOff()) synth->allNotesOff();
    else if (m.isController()) synth->midiCC (m.getControllerNumber(), m.getControllerValue());
    else if (m.isPitchWheel()) synth->bend (m.getPitchWheelValue());
    else if (m.isChannelPressure()) synth->aftertouch (m.getChannelPressureValue() / 127.0);
}
void AlbedoProcessor::processRegion (juce::AudioBuffer<float>& b, int start, int length)
{
    // Stereo write pointers address the exact host region; no waveform copies.
    if (length > 0) synth->process (b.getWritePointer (0, start), b.getWritePointer (1, start), (uint32_t) length);
}
void AlbedoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    updateSynthParameters();
    int pos = 0;
    for (const auto metadata : midi)
    {
        const auto step = juce::jlimit (pos, buffer.getNumSamples(), metadata.samplePosition);
        processRegion (buffer, pos, step - pos);
        pos = step;
        handleMidi (metadata.getMessage());
    }
    processRegion (buffer, pos, buffer.getNumSamples() - pos);
    renderRear(buffer);
    activeVoiceCount.store (synth->activeVoices(), std::memory_order_relaxed);
}
void AlbedoProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    updateSynthParameters();
    int pos=0;
    for (const auto metadata : midi)
    {
        const int step=juce::jlimit(pos,buffer.getNumSamples(),metadata.samplePosition);
        if (step>pos) synth->process64(buffer.getWritePointer(0,pos),buffer.getWritePointer(1,pos),(uint32_t)(step-pos));
        pos=step;
        handleMidi(metadata.getMessage());
    }
    if (buffer.getNumSamples()>pos)
        synth->process64(buffer.getWritePointer(0,pos),buffer.getWritePointer(1,pos),
                         (uint32_t)(buffer.getNumSamples()-pos));
    renderRear(buffer);
    activeVoiceCount.store(synth->activeVoices(),std::memory_order_relaxed);
}
juce::AudioParameterFloat* AlbedoProcessor::parameter (uint32_t id) const noexcept
{
    const auto it = idParameters.find (id); return it == idParameters.end() ? nullptr : it->second;
}
float AlbedoProcessor::paramValue (uint32_t id) const noexcept
{
    if (auto* p = parameter (id)) return p->get(); return 0;
}
void AlbedoProcessor::setCurrentProgram (int index) { loadFactoryPreset (index); }
const juce::String AlbedoProcessor::getProgramName (int i)
{
    if (i < 0 || i >= getNumPrograms()) return {};
    if (i < (int) albedo_factory::patches.size())
        return juce::String (albedo_factory::patches[(size_t)i].name);
    return juce::String (albedo_creative::patches[(size_t)(i-64)].name);
}
void AlbedoProcessor::loadFactoryPreset (int index)
{
    if (index < 0 || index >= getNumPrograms()) return;
    albedo_programs::apply(index, [this](uint32_t id, float value) {
        if (auto* p=parameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(p->range.start,p->range.end,value)));
    });
    currentProgram.store (index);
}
void AlbedoProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream out (dest, false);
    out.writeInt (0x414C4232); // ALB2
    out.writeInt (currentProgram.load());
    out.writeInt ((int) parameterList.size());
    for (const auto& [id, param] : parameterList) { out.writeInt ((int) id); out.writeDouble (param->get()); }
}
void AlbedoProcessor::setStateInformation (const void* data, int size)
{
    juce::MemoryInputStream in (data, (size_t) juce::jmax (0, size), false);
    if (in.readInt() != 0x414C4232) return;
    currentProgram.store (juce::jlimit (0, getNumPrograms()-1, in.readInt()));
    const int count = juce::jlimit (0, 500, in.readInt());
    for (int i=0; i<count && !in.isExhausted(); ++i)
    {
        const auto id = (uint32_t) in.readInt(); const auto v = (float) in.readDouble();
        if (auto* p = parameter (id)) p->setValueNotifyingHost (p->convertTo0to1 (v));
    }
}
juce::AudioProcessorEditor* AlbedoProcessor::createEditor()
{
    // Fail over to the fully working JUCE-native 3:2 editor if WebView2 is not
    // supported; never allow an IE fallback to run the live CP31 interface.
    if(AlbedoWebViewEditor::isAvailable())
        return new AlbedoWebViewEditor(*this);
    return new AlbedoEditor(*this);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new AlbedoProcessor(); }
