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
    pendingValues=lastValues; // Preallocate every source parameter snapshot before audio starts.
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
    outputGuard.prepare(sampleRate);
    setLatencySamples(AlbedoOutputGuard::latency); // Fixed, accurately reported 63-sample lookahead
    panicVoices();
    parametersChanged.store(true, std::memory_order_release);
    updateSynthParameters();
}
void AlbedoProcessor::releaseResources() { panicVoices(); outputGuard.reset(); }
bool AlbedoProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{ return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() || l.getMainOutputChannelSet() == juce::AudioChannelSet::quadraphonic(); }
void AlbedoProcessor::updateSynthParameters()
{
    if (!parametersChanged.exchange(false, std::memory_order_acq_rel)) return;
    const unsigned version=presetGeneration.load(std::memory_order_acquire);
    if(version&1u) { parametersChanged.store(true,std::memory_order_release); return; }
    // Snapshot all parameters first. Do not apply half of a GUI preset swap.
    // Both vectors were allocated in the constructor, never on the audio thread.
    for(size_t i=0;i<parameterList.size();++i)
        pendingValues[i]=double(parameterList[i].second->get());
    if(presetGeneration.load(std::memory_order_acquire)!=version) {
        parametersChanged.store(true,std::memory_order_release);return;
    }
    for(size_t i=0;i<parameterList.size();++i) {
        const double val=pendingValues[i];
        if(std::isfinite(val)&&val!=lastValues[i]) {
            synth->setParam(parameterList[i].first,val);lastValues[i]=val;
        }
    }
}
void AlbedoProcessor::panicVoices() noexcept
{
    synth->allNotesOff();
    for(auto& channel:midiHeld)channel.fill(false);
}
void AlbedoProcessor::syncArpClock()
{
    if (auto* transport=getPlayHead()) {
        if(auto position=transport->getPosition()) {
            if(auto bpm=position->getBpm()) synth->setArpTempo(*bpm);
            const bool playing=position->getIsPlaying();
            if(transportPlaying && !playing) panicVoices(); // Stop clears all voices, even without the arpeggiator
            transportPlaying=playing;
        }
    }
}
void AlbedoProcessor::handleMidi (const juce::MidiMessage& m)
{
    const int ch=juce::jlimit(0,15,m.getChannel()-1);
    if(m.isNoteOn()) {
        const int note=m.getNoteNumber();
        if(midiHeld[(size_t)ch][(size_t)note]) synth->noteOff(note,ch); // Retrigger: never accumulate lost same-key voices.
        midiHeld[(size_t)ch][(size_t)note]=true;
        synth->noteOn(note,m.getFloatVelocity(),ch);
    }
    else if(m.isNoteOff()) {
        const int note=m.getNoteNumber();
        midiHeld[(size_t)ch][(size_t)note]=false;
        synth->noteOff(note,ch);
    }
    else if(m.isAllNotesOff() || m.isAllSoundOff() || m.isMidiStop()) panicVoices();
    else if(m.isController()) {
        const int cc=m.getControllerNumber();
        if(cc==120||cc==123){panicVoices();return;}
        synth->midiCC(cc,m.getControllerValue(),ch); // Sustain and release belong to the event's MIDI channel.
    }
    else if(m.isPitchWheel()) synth->bend(m.getPitchWheelValue());
    else if(m.isChannelPressure()) synth->aftertouch(m.getChannelPressureValue()/127.0);
}
void AlbedoProcessor::processRegion (juce::AudioBuffer<float>& b, int start, int length)
{
    // Stereo write pointers address the exact host region; no waveform copies.
    if (length > 0) synth->process (b.getWritePointer (0, start), b.getWritePointer (1, start), (uint32_t) length);
}
void AlbedoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    if(pendingPanic.exchange(false,std::memory_order_acq_rel)) panicVoices();
    syncArpClock();
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
    outputGuard.process(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples());
    renderRear(buffer);
    activeVoiceCount.store (synth->activeVoices(), std::memory_order_relaxed);
}
void AlbedoProcessor::processBlock (juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    if(pendingPanic.exchange(false,std::memory_order_acq_rel)) panicVoices();
    syncArpClock();
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
    outputGuard.process(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples());
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
    if(i<128) return juce::String (albedo_creative::patches[(size_t)(i-64)].name);
    return juce::String (albedo_morph::patches[(size_t)(i-128)].name);
}
void AlbedoProcessor::loadFactoryPreset (int index)
{
    if(index<0 || index>=getNumPrograms())return;
    presetGeneration.fetch_add(1u,std::memory_order_acq_rel); // odd: coherent preset write underway
    albedo_programs::apply(index,[this](uint32_t id,float value){
        if(auto* p=parameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(p->range.start,p->range.end,value)));
    });
    currentProgram.store(index,std::memory_order_release);
    presetGeneration.fetch_add(1u,std::memory_order_release); // even: complete
    parametersChanged.store(true,std::memory_order_release);
    pendingPanic.store(true,std::memory_order_release); // Old release/sustain may not cross new preset.
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
    presetGeneration.fetch_add(1u,std::memory_order_acq_rel);
    currentProgram.store (juce::jlimit (0, getNumPrograms()-1, in.readInt()));
    const int count = juce::jlimit (0, 500, in.readInt());
    for (int i=0; i<count && !in.isExhausted(); ++i)
    {
        const auto id = (uint32_t) in.readInt(); const auto v = (float) in.readDouble();
        if(auto* p=parameter(id);p&&std::isfinite(v))
            p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(p->range.start,p->range.end,v)));
    }
    presetGeneration.fetch_add(1u,std::memory_order_release);
    parametersChanged.store(true,std::memory_order_release);
    pendingPanic.store(true,std::memory_order_release);
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
