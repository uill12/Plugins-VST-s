#include "AlbedoPrograms.hpp"
#include "vitriol_dsp.hpp"
#include "albedo_factory_patches.hpp"
#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    static_assert(albedo_factory::patches.size()==64);
    static_assert(albedo_creative::patches.size()==64);
    static_assert(albedo_morph::patches.size()==64);
    bool ok=true;
    int audible=0;
    for (int index=0; index<192; ++index)
    {
        auto s=std::make_unique<vitriol::Synth>();
        albedo_programs::apply(index,[&](uint32_t id,float value){ s->setParam(id,value); });
        const int sr=index%2==0?48000:88200;
        s->setParam(6020,0); // Verify the polyphonic instrument beneath each arp program.
        s->prepare(sr);
        for(int n=48;n<60;++n)s->noteOn(n,.57);
        const bool poly=s->activeVoices()>=12;
        const size_t N=static_cast<size_t>(sr/4);
        std::vector<double> left(N),right(N);
        s->process64(left.data(),right.data(),(uint32_t)N);
        double energy=0,peak=0;
        bool finite=true;
        for(size_t j=0;j<N;++j)
        {
            finite&=std::isfinite(left[j])&&std::isfinite(right[j]);
            energy+=(left[j]*left[j]+right[j]*right[j])*.5;
            peak=std::max(peak,std::max(std::abs(left[j]),std::abs(right[j])));
        }
        energy/=N;
        const bool good=finite && poly && peak<=1.000001;
        ok&=good && (energy>1.0e-13);
        audible+=energy>1.0e-13;
        // Test retrigger and note-off without stale voices/non-finite output.
        for(int n=48;n<60;++n)s->noteOff(n);
        s->process64(left.data(),right.data(),(uint32_t)N);
        for(size_t j=0;j<N;++j)ok&=std::isfinite(left[j])&&std::isfinite(right[j]);
        std::cout<< "CP51 program " <<index
                 <<" rate "<<sr<<"Hz poly "<<poly
                 <<" RMS "<<std::sqrt(energy)<<" peak "<<peak
                 <<" finite "<<finite<<"\n";
    }
    std::cout<<"CP51 audible programs "
             <<audible<<"/192; finite/polyphonic "<<ok<<"\n";
    return ok && audible==192 ? 0 : 1;
}
