#include "AlbedoCreativeBank.hpp"
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
    bool ok=true;
    int audible=0;
    for (const auto& patch:albedo_creative::patches)
    {
        auto s=std::make_unique<vitriol::Synth>();
        const auto& base=albedo_factory::patches[static_cast<size_t>(patch.baseIndex)];
        for(size_t i=0;i<albedo_factory::parameterIds.size();++i)
        {
            auto id=albedo_factory::parameterIds[i];
            s->setParam(id,id==3111?0.0:base.values[i]);
        }
        for(size_t i=0;i<patch.changeCount;++i)
            s->setParam(patch.changes[i].id,patch.changes[i].id==3111?0:patch.changes[i].value);
        const int sr=patch.index%2==0?48000:88200;
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
        std::cout<<"CP38 program "<<patch.index<<" "<<patch.name
                 <<" rate "<<sr<<"Hz poly "<<poly
                 <<" RMS "<<std::sqrt(energy)<<" peak "<<peak
                 <<" finite "<<finite<<"\n";
    }
    std::cout<<"Original 64 unchanged; 64 additional voices; audible "
             <<audible<<"/64; finite/polyphonic "<<ok<<"\n";
    return ok && audible==64 ? 0 : 1;
}
