#include "AlbedoProcessor.h"
#include "AlbedoEditor.h"
#include "AlbedoWebViewEditor.h"
#include <cmath>
#include <limits>
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
    nextCallback={};
    midiIdleGuard.arm(albedo::MidiIdleGuard::Clock::now());
    synth->prepare (sampleRate);
    juce::ignoreUnused(samplesPerBlock);
    rearField.prepare(sampleRate);
    outputGuard.prepare(sampleRate);
    setLatencySamples(AlbedoOutputGuard::latency); // Fixed, accurately reported 63-sample lookahead
    panicVoices();
    parametersChanged.store(true, std::memory_order_release);
    updateSynthParameters();
}
void AlbedoProcessor::releaseResources() { reset(); }
void AlbedoProcessor::reset() { resumeReset.store(true,std::memory_order_release); }

void AlbedoProcessor::recoverInterruptedAudio(int frames)
{
    using Clock=std::chrono::steady_clock;
    bool interrupted=resumeReset.exchange(false,std::memory_order_acq_rel);
    if(isNonRealtime()) nextCallback={};
    else {
        const auto now=Clock::now();
        interrupted |= nextCallback!=Clock::time_point{} && now-nextCallback>std::chrono::seconds(2);
        const std::chrono::duration<double> duration(frames/std::max(1.0,getSampleRate()));
        nextCallback=now+std::chrono::duration_cast<Clock::duration>(duration);
        // When the host continues callbacks during MIDI silence, clear
        // abandoned DSP/FX state in an IDLE block, before the next note.
        // This prevents reset work from delaying the first new key. A real
        // held key or sustain pedal keeps the instrument alive indefinitely.
        if(!interrupted && midiIdleGuard.lastMidi!=Clock::time_point{} &&
           now>midiIdleGuard.lastMidi &&
           now-midiIdleGuard.lastMidi>albedo::MidiIdleGuard::idleLimit){
            bool held=false,pedal=false;
            for(const auto& channel:midiHeld)for(bool down:channel)held|=down;
            for(bool down:midiPedalDown)pedal|=down;
            if(!held&&!pedal)interrupted=true;
        }
    }
    if(interrupted){
        // CP59: a suspended ASIO callback must not resurrect an orphaned held
        // voice or 18-second wet tail when the host resumes.
        synth->reset();
        for(auto& channel:midiHeld) channel.fill(false);
        for(auto& stamps:midiPressStamp) stamps.fill(0);
        midiPedalDown.fill(false);
        midiIdleGuard.arm(Clock::now());
        transportPlaying=false;
        outputGuard.reset();
    }
}
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
    for(auto& stamps:midiPressStamp)stamps.fill(0);
    midiPedalDown.fill(false);
}
void AlbedoProcessor::syncArpClock()
{
    // Position belongs to the host. MIDI subregions are sample-accurate in DSP.
    double ppq=std::numeric_limits<double>::quiet_NaN();
    bool playing=false;
    float tempo=0.f;
    if(auto* transport=getPlayHead()){
        if(auto position=transport->getPosition()){
            if(auto bpm=position->getBpm(); bpm && std::isfinite(*bpm) && *bpm>0)
                tempo=static_cast<float>(juce::jlimit(20.0,400.0,*bpm));
            playing=position->getIsPlaying();
            if(auto beat=position->getPpqPosition())ppq=*beat;
            if(transportPlaying&&!playing){
                synth->reset(); // Stop stale arpeggiator/sustain/FX state together.
                for(auto& channel:midiHeld)channel.fill(false);
                for(auto& stamps:midiPressStamp)stamps.fill(0);
                midiPedalDown.fill(false);
                midiIdleGuard.arm(albedo::MidiIdleGuard::Clock::now());
                outputGuard.reset();
            }
            transportPlaying=playing;
        }
    }
    hostTempo.store(tempo,std::memory_order_relaxed);
    synth->setArpTempo(tempo);
    synth->setArpTransport(ppq,playing);
}
void AlbedoProcessor::handleMidi (const juce::MidiMessage& m)
{
    const int ch=juce::jlimit(0,15,m.getChannel()-1);
    if(m.isNoteOn()) {
        // Restore clean DSP ownership at the FIRST new key after MIDI inactivity.
        // Crucially, do not cut legitimate long-held notes, sustained chords,
        // including when the DAW transport is running. Applies with ARP OFF.
        bool held=false,pedal=false;
        for(const auto& channel:midiHeld)for(bool down:channel)held|=down;
        for(bool down:midiPedalDown)pedal|=down;
        const bool rearm=midiIdleGuard.onNoteOn(
            albedo::MidiIdleGuard::Clock::now(),held,pedal);
        if(!isNonRealtime()&&rearm) {
            synth->reset();
            outputGuard.reset();
            for(auto& channel:midiHeld)channel.fill(false);
            for(auto& stamps:midiPressStamp)stamps.fill(0);
            midiPedalDown.fill(false);
        }
        const int note=m.getNoteNumber();
        if(midiHeld[(size_t)ch][(size_t)note])
            synth->noteOff(note,ch); // A retrigger replaces one held key.
        midiHeld[(size_t)ch][(size_t)note]=true;
        midiPressStamp[(size_t)ch][(size_t)note]=++nextMidiPressStamp;
        synth->noteOn(note,m.getFloatVelocity(),ch);
    }
    else if(m.isNoteOff()) {
        midiIdleGuard.onOtherMidi(albedo::MidiIdleGuard::Clock::now());
        const int note=m.getNoteNumber();
        int owner=ch;
        if(!midiHeld[(size_t)ch][(size_t)note]) {
            // A reconnecting MIDI bridge may send the release on a different
            // channel from the press. Select the oldest outstanding press of
            // this exact pitch; never release a different pitch or whole chord.
            uint64_t oldest=std::numeric_limits<uint64_t>::max();
            for(int candidate=0;candidate<16;++candidate)
                if(midiHeld[(size_t)candidate][(size_t)note] &&
                   midiPressStamp[(size_t)candidate][(size_t)note]<oldest){
                    oldest=midiPressStamp[(size_t)candidate][(size_t)note];
                    owner=candidate;
                }
        }
        midiHeld[(size_t)owner][(size_t)note]=false;
        midiPressStamp[(size_t)owner][(size_t)note]=0;
        synth->noteOff(note,owner);
    }
    else if(m.isAllSoundOff() || m.isMidiStop()){
        midiIdleGuard.onOtherMidi(albedo::MidiIdleGuard::Clock::now());
        synth->reset();for(auto& held:midiHeld)held.fill(false);
        for(auto& stamps:midiPressStamp)stamps.fill(0);
        midiPedalDown.fill(false);
        outputGuard.reset();
    }
    else if(m.isAllNotesOff()){
        midiIdleGuard.onOtherMidi(albedo::MidiIdleGuard::Clock::now());
        synth->midiCC(123,0,ch);midiHeld[(size_t)ch].fill(false);
        midiPressStamp[(size_t)ch].fill(0);midiPedalDown[(size_t)ch]=false;
    }
    else if(m.isController()) {
        midiIdleGuard.onOtherMidi(albedo::MidiIdleGuard::Clock::now());
        const int cc=m.getControllerNumber();
        if(cc==120){synth->reset();for(auto& held:midiHeld)held.fill(false);for(auto& stamps:midiPressStamp)stamps.fill(0);midiPedalDown.fill(false);outputGuard.reset();return;}
        if(cc==123){synth->midiCC(123,0,ch);midiHeld[(size_t)ch].fill(false);midiPressStamp[(size_t)ch].fill(0);midiPedalDown[(size_t)ch]=false;return;}
        if(cc==64)midiPedalDown[(size_t)ch]=m.getControllerValue()>=64;
        if(cc==121)midiPedalDown[(size_t)ch]=false;
        // CC121 resets controller state, including an orphaned sustain pedal.
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
    recoverInterruptedAudio(buffer.getNumSamples());
    if(pendingPanic.exchange(false,std::memory_order_acq_rel)) panicVoices();
    updateSynthParameters();
    syncArpClock();
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
    recoverInterruptedAudio(buffer.getNumSamples());
    if(pendingPanic.exchange(false,std::memory_order_acq_rel)) panicVoices();
    updateSynthParameters();
    syncArpClock();
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
    if(auto* meter=parameter(6032)) meter->setValueNotifyingHost(0.f);
    if(auto* wolfram=parameter(6031)) wolfram->setValueNotifyingHost(0.0f);
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
