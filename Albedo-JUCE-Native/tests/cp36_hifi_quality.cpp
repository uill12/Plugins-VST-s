#include "vitriol_dsp.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

int main()
{
    bool ok=true;
    constexpr int rates[]={44100,48000,88200,96000};
    constexpr int methods[]={0,4,7};
    for(int rate:rates) for(int method:methods)
    {
        auto a=std::make_unique<vitriol::Synth>();
        auto b=std::make_unique<vitriol::Synth>();
        for(auto* s:{a.get(),b.get()})
        {
            s->setParam(1000,method);
            s->setParam(1033,0);s->setParam(1065,0);s->setParam(1097,0);
            s->setParam(9002,0);s->setParam(5005,0);s->setParam(5003,0);
            s->prepare(rate);s->noteOn(67,.76);
        }
        std::vector<float> fl(2111),fr(2111);
        std::vector<double> dl(2111),dr(2111);
        a->process(fl.data(),fr.data(),(uint32_t)fl.size());
        b->process64(dl.data(),dr.data(),(uint32_t)dl.size());
        double diff=0,residue=0,peak=0,rms=0;
        for(size_t i=0;i<dl.size();i++)
        {
            if(!std::isfinite(dl[i])||!std::isfinite(dr[i]))ok=false;
            diff=std::max({diff,std::abs(dl[i]-double(fl[i])),std::abs(dr[i]-double(fr[i]))});
            residue=std::max(residue,std::abs(dl[i]-double(float(dl[i]))));
            peak=std::max({peak,std::abs(dl[i]),std::abs(dr[i])});
            rms+=(dl[i]*dl[i]+dr[i]*dr[i])*.5;
        }
        rms=std::sqrt(rms/(double)dl.size());
        const bool pass=diff<1e-6&&residue>1e-12&&peak<=1.000001&&rms>1e-6;
        ok &=pass;
        std::cout<<rate<<"Hz method="<<method<<" max32/64="<<diff
                 <<" doubleResidue="<<residue<<" peak="<<peak
                 <<" rms="<<rms<<" passed="<<pass<<"\n";
    }
    auto synth=std::make_unique<vitriol::Synth>();
    synth->prepare(48000);
    for(int note=48;note<60;note++)synth->noteOn(note,.7);
    std::vector<double> left(8192),right(8192);
    synth->process64(left.data(),right.data(),(uint32_t)left.size());
    ok &=synth->activeVoices()>=12;
    for(size_t i=0;i<left.size();i++)
        ok &=std::isfinite(left[i])&&std::isfinite(right[i]);
    std::cout<<"12 voices, 8192 samples, finite="<<ok<<"\n";
    return ok?0:1;
}
