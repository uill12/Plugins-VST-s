// CP59 DSP quality regression. Public API only. Exit code 0 = pass.
// Covers: physical-string tuning, click-free voice reuse, phase-continuous chorus,
// stereo/stable FDN reverb at every host rate, glitch-free Size automation.
#include "vitriol_dsp.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <numeric>
#include <vector>
using vitriol::Synth;
static bool g_ok=true;
static void check(bool ok,const char* what,double value,double limit){
    std::printf("  [%s] %-58s value=% .5g limit=% .5g\n",ok?"PASS":"FAIL",what,value,limit);
    g_ok&=ok;
}
static std::unique_ptr<Synth> makeSine(int sr){
    auto s=std::make_unique<Synth>();s->prepare(sr);
    for(int e=0;e<4;e++){s->setParam(1000+32*e+2,e==0?100:0);s->setParam(1000+32*e+1,e==0?1:0);}
    s->setParam(1000,2);s->setParam(1016,1);s->setParam(1019,0);s->setParam(1006,50);s->setParam(1009,0);
    for(int i=0;i<4;i++)s->setParam(5001+2*i,0);
    s->setParam(3000,20000);s->setParam(3003,20000);s->setParam(3001,0);s->setParam(3004,0);
    s->setParam(3002,0);s->setParam(3005,0);s->setParam(3100,.01);s->setParam(3102,100);s->setParam(9001,0);
    s->setParam(6020,0);
    return s;
}
static double maxD2(const std::vector<double>& x,size_t a,size_t b){
    double m=0;for(size_t i=a+2;i<b&&i<x.size();++i)m=std::max(m,std::abs(x[i]-2*x[i-1]+x[i-2]));return m;
}
static double acfPitch(const std::vector<double>& x,int sr){
    std::vector<double> y(x.begin()+sr/20,x.begin()+sr*9/20);const double mean=std::accumulate(y.begin(),y.end(),0.)/y.size();
    for(double& v:y)v-=mean;
    const int lmin=sr/4000,lmax=sr/50;std::vector<double> ac(lmax+2);
    for(int l=0;l<=lmax+1;++l){double a=0;for(size_t i=0;i+l<y.size();++i)a+=y[i]*y[i+l];ac[l]=a;}
    double best=0;for(int l=lmin;l<=lmax;++l)best=std::max(best,ac[l]/ac[0]);
    for(int l=lmin+1;l<lmax;++l)if(ac[l]>ac[l-1]&&ac[l]>=ac[l+1]&&ac[l]/ac[0]>=.85*best){
        const double d=.5*(ac[l-1]-ac[l+1])/(ac[l-1]-2*ac[l]+ac[l+1]);return sr/(l+d);}
    return 0;
}
int main(){
    // 1. Physical string: pitch accuracy across seven octaves
    std::printf("Physical / Modal string tuning\n");
    {
        constexpr int sr=48000;double worst=0;
        for(int note=36;note<=102;note+=6){
            auto s=makeSine(sr);s->setParam(1000,5);s->setParam(3103,.5);s->noteOn(note,.8);
            std::vector<double> l(sr),r(sr);for(int p=0;p<sr;p+=256)s->process64(&l[p],&r[p],std::min(256,sr-p));
            const double f0=440*std::pow(2.,(note-69)/12.),fm=acfPitch(l,sr);
            worst=std::max(worst,std::abs(1200*std::log2(std::max(fm,1.)/f0)));
        }
        check(worst<=2.5,"max |pitch error| MIDI 36-102 (cents)",worst,2.5);
    }
    // 2. Voice reuse is click-free (steal the quietest tail, keep waveform continuity)
    std::printf("Voice stealing / retrigger continuity\n");
    for(int mode=0;mode<2;mode++){
        constexpr int sr=48000;auto s=makeSine(sr);s->setParam(3100,.01);s->setParam(3103,3.);
        std::vector<double> l(sr*6),r(sr*6);size_t pos=0;
        std::vector<std::pair<size_t,int>> on,off;
        if(mode==0)for(int k=0;k<24;k++){on.push_back({size_t(sr*(.1+.06*k)),48+(k*5)%24});off.push_back({size_t(sr*(.22+.06*k)),48+(k*5)%24});}
        else for(int k=0;k<6;k++){on.push_back({size_t(sr*(.1+.35*k)),60});off.push_back({size_t(sr*(.3+.35*k)),60});}
        while(pos<l.size()){const size_t next=std::min(l.size(),pos+64);
            for(auto& e:on)if(e.first>=pos&&e.first<next)s->noteOn(e.second,.8);
            for(auto& e:off)if(e.first>=pos&&e.first<next)s->noteOff(e.second);
            s->process64(&l[pos],&r[pos],uint32_t(next-pos));pos=next;}
        const double m=maxD2(l,size_t(sr*.2),l.size());
        check(m<=.0025,mode==0?"steal: max second difference (click size)":"retrigger: max second difference (click size)",m,.0025);
    }
    // 3. Chorus rate automation must not jump the delay phase (six rate changes at unrelated times)
    std::printf("Chorus phase continuity\n");
    {
        constexpr int sr=48000;auto s=makeSine(sr);s->setParam(5003,1);s->setParam(5002,100);s->setParam(5008,.17);
        std::vector<double> l(sr*24),r(sr*24);s->noteOn(57,.8);size_t pos=0;int next=0;
        std::vector<size_t> flips;for(int k=0;k<6;k++)flips.push_back(size_t(sr*(10.37+1.41*k)));
        while(pos<l.size()){if(next<6&&pos>=flips[next]){s->setParam(5008,next%2?.17:3.0);++next;}s->process64(&l[pos],&r[pos],64);pos+=64;}
        const double steady=maxD2(l,sr*5,sr*10);double worst=0;
        for(size_t f:flips)worst=std::max(worst,maxD2(l,f,f+2048));
        check(worst<=steady*1.3,"max d2 right after rate changes / steady state (ratio)",worst/steady,1.3);
    }
    // 4. Reverb: stereo spread, stability and decay at every host rate, extreme settings
    std::printf("Stereo FDN reverb\n");
    for(int sr:{44100,48000,96000,192000}){
        auto s=makeSine(sr);s->setParam(1000,6);s->setParam(1004,-100); // noise source panned hard left
        s->setParam(5005,1);s->setParam(5004,100);s->setParam(5010,0);s->setParam(5011,140);s->setParam(2006,100);
        s->noteOn(48,1.);
        const size_t N=size_t(sr)*16;std::vector<double> l(N),r(N);size_t pos=0;bool finite=true;
        bool released=false;
        while(pos<N){if(!released&&pos>=size_t(sr)*3){s->noteOff(48);released=true;}const size_t n=std::min<size_t>(512,N-pos);s->process64(&l[pos],&r[pos],uint32_t(n));pos+=n;}
        for(size_t i=0;i<N;i++)finite&=std::isfinite(l[i])&&std::isfinite(r[i]);
        auto rms=[&](const std::vector<double>& x,double a,double b){double e=0;size_t i0=size_t(a*sr),i1=size_t(b*sr);for(size_t i=i0;i<i1;i++)e+=x[i]*x[i];return std::sqrt(e/(i1-i0));};
        const double spread=rms(r,3.5,6.5)/std::max(rms(l,3.5,6.5),1e-12);
        const double late=20*std::log10(std::max(rms(l,14,16),1e-12)/std::max(rms(l,3.5,4.0),1e-12));
        char name[96];
        std::snprintf(name,sizeof name,"%d Hz finite and bounded",sr);check(finite,name,finite,1);
        std::snprintf(name,sizeof name,"%d Hz right/left tail RMS for a hard-left source",sr);check(spread>=.35,name,spread,.35);
        std::snprintf(name,sizeof name,"%d Hz tail decays (dB re 3.5 s) at max size/duration",sr);check(late<=-30,name,late,-30);
    }
    // 5. Size automation glides (no delay-line jump)
    std::printf("Reverb Size automation\n");
    {
        constexpr int sr=48000;auto s=makeSine(sr);s->setParam(5005,1);s->setParam(5004,60);s->setParam(5011,60);
        s->noteOn(55,.8);std::vector<double> l(sr*8),r(sr*8);size_t pos=0;
        while(pos<l.size()){if(pos>=size_t(sr*3)&&pos<size_t(sr*6))s->setParam(5011,60+80.*(double(pos)-sr*3)/(sr*3));s->process64(&l[pos],&r[pos],256);pos+=256;}
        const double still=maxD2(l,sr*1,sr*3-512),sweep=maxD2(l,sr*3,sr*6);
        check(sweep<=still*3+1e-4,"max d2 during Size sweep vs steady state (ratio)",sweep/std::max(still,1e-9),3.);
    }
    std::printf(g_ok?"CP59 DSP QUALITY PASS\n":"CP59 DSP QUALITY FAIL\n");
    return g_ok?0:1;
}