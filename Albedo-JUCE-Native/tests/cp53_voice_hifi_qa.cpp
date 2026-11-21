#include "AlbedoPrograms.hpp"
#include "AlbedoOutputGuard.hpp"
#include "vitriol_dsp.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>

int main()
{
    bool pass=true;
    constexpr int rates[]{44100,48000,88200,96000,176400,192000};
    int activePrograms=0,programsWithLimiting=0;
    double maxPeak=0,maxRealtimeRatio=0;
    // Exercise every CP51 program through all four oscillator sources, the
    // Corpus filters and FX, in twelve-note chords at multiple host rates.
    for(int preset=0;preset<192;++preset) {
        auto synth=std::make_unique<vitriol::Synth>();
        albedo_programs::apply(preset,[&](uint32_t id,float value){synth->setParam(id,value);});
        synth->setParam(6020,0); // Verify ordinary polyphony separately from arpeggiated note events.
        const int sr=rates[(preset/12)%6];
        synth->prepare(sr);
        AlbedoOutputGuard guard;guard.prepare(sr);
        for(int n=48;n<60;++n)synth->noteOn(n,.70,n%3);
        const bool poly=synth->activeVoices()>=12;
        std::array<double,2048> l{},r{};
        const auto start=std::chrono::steady_clock::now();
        synth->process64(l.data(),r.data(),(uint32_t)l.size());
        guard.process(l.data(),r.data(),(int)l.size());
        const double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
        maxRealtimeRatio=std::max(maxRealtimeRatio,seconds/(double(l.size())/sr));
        double peak=0,energy=0;bool finite=true;
        for(size_t i=0;i<l.size();++i){
            peak=std::max({peak,std::abs(l[i]),std::abs(r[i])});
            energy+=(l[i]*l[i]+r[i]*r[i])*.5;
            finite=finite&&std::isfinite(l[i])&&std::isfinite(r[i]);
        }
        maxPeak=std::max(peak,maxPeak);
        activePrograms+=(energy>1.0e-13);
        programsWithLimiting+=(guard.currentGain()<.999);
        const bool good=poly&&finite&&peak<=.8800001&&seconds<5.0;
        if(!good)std::cerr<<"CP53 FAIL preset="<<preset<<" sr="<<sr
                 <<" poly="<<poly<<" peak="<<peak<<" seconds="<<seconds<<"\n";
        pass &=good;
        for(int n=48;n<60;++n)synth->noteOff(n,n%3);
        synth->allSoundOff();
        pass&=synth->activeVoices()==0;
    }
    // Sustain is channel-local; normal release follows the preset envelope,
    // whereas all-notes-off releases safely within ~120 ms, even for long pads.
    for(int sr:rates) {
        auto synth=std::make_unique<vitriol::Synth>();
        synth->setParam(3103,.18);
        synth->prepare(sr);synth->setParam(6020,0);
        synth->noteOn(60,.8,2);synth->noteOn(60,.8,5);
        synth->midiCC(64,127,5);
        synth->noteOff(60,2);synth->noteOff(60,5);
        std::array<double,256> l{},r{};
        // Hold long enough for channel 2's ordinary release to finish.
        // Only the pedal-held channel 5 voice should remain active.
        for(int i=0;i<int(sr*1.0/256+1);++i)synth->process64(l.data(),r.data(),256);
        const bool sustain=synth->activeVoices()==1;
        synth->midiCC(64,0,5);
        for(int i=0;i<int(sr*1.0/256+1);++i)synth->process64(l.data(),r.data(),256);
        const bool released=synth->activeVoices()==0;
        synth->setParam(3103,18.0);
        synth->noteOn(55,.8,5);
        for(int i=0;i<10;++i)synth->process64(l.data(),r.data(),256);
        synth->allNotesOff();
        for(int i=0;i<int(sr*.3/256+1);++i)synth->process64(l.data(),r.data(),256);
        const bool panicReleased=synth->activeVoices()==0;
        synth->setParam(6020,1);synth->setParam(6030,0);
        synth->noteOn(50,.7,5);synth->noteOn(57,.7,5);
        for(int i=0;i<20;++i)synth->process64(l.data(),r.data(),256);
        synth->noteOff(50,5);synth->noteOff(57,5);
        synth->allNotesOff();
        for(int i=0;i<int(sr*.3/256+1);++i)synth->process64(l.data(),r.data(),256);
        const bool arpSafe=synth->activeVoices()==0;
        pass &=sustain&&released&&panicReleased&&arpSafe;
        std::cout<<"rate="<<sr<<" sustain="<<sustain
                 <<" normalRelease="<<released<<" panicRelease="<<panicReleased
                 <<" arpRelease="<<arpSafe<<"\n";
    }
    std::cout<<"CP53 192-program audit: audible="<<activePrograms
             <<" limitedPrograms="<<programsWithLimiting
             <<" maxSamplePeak="<<maxPeak<<" maxRealtimeLoad="<<maxRealtimeRatio
             <<" pass="<<pass<<"\n";
    return pass?0:1;
}
