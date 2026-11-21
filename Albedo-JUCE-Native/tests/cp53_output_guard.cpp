#include "AlbedoOutputGuard.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

int main()
{
    bool ok=true;
    constexpr int rates[]{44100,48000,88200,96000,176400,192000};
    for(const int sr:rates)
    {
        constexpr int n=16384;
        std::vector<double> l(n),r(n),fullL(n),fullR(n),splitL(n),splitR(n);
        for(int i=0;i<n;++i){
            const double t=double(i)/sr;
            l[i]=.31*std::sin(2*3.14159265358979323846*110*t)+
                 .22*std::sin(2*3.14159265358979323846*1873*t);
            r[i]=.27*std::sin(2*3.14159265358979323846*127*t)+
                 .21*std::sin(2*3.14159265358979323846*1951*t);
        }
        l[107]=5.0;r[108]=-7.0;
        l[800]=std::numeric_limits<double>::quiet_NaN();
        r[802]=std::numeric_limits<double>::infinity();
        fullL=l;fullR=r;splitL=l;splitR=r;
        AlbedoOutputGuard all,blocks;
        all.prepare(sr);blocks.prepare(sr);
        all.process(fullL.data(),fullR.data(),n);
        int pos=0;
        while(pos<n){const int count=std::min((pos%71)+1,n-pos);
            blocks.process(splitL.data()+pos,splitR.data()+pos,count);pos+=count;}
        double diff=0,peak=0,energy=0;
        for(int i=0;i<n;++i)
        {
            diff=std::max({diff,std::abs(fullL[i]-splitL[i]),std::abs(fullR[i]-splitR[i])});
            peak=std::max({peak,std::abs(fullL[i]),std::abs(fullR[i])});
            energy+=fullL[i]*fullL[i]+fullR[i]*fullR[i];
            if(!std::isfinite(fullL[i])||!std::isfinite(fullR[i]))ok=false;
        }
        AlbedoOutputGuard clean;clean.prepare(sr);
        std::vector<double> quietL(n),quietR(n);
        for(int i=0;i<n;++i){quietL[i]=.2*std::sin(2*3.14159265358979323846*80*i/sr);quietR[i]=quietL[i]*-.75;}
        clean.process(quietL.data(),quietR.data(),n);
        double cleanPeak=0,stereoDiff=0;
        for(int i=512;i<n;++i){cleanPeak=std::max(cleanPeak,std::abs(quietL[i]));
            stereoDiff=std::max(stereoDiff,std::abs(quietR[i]+quietL[i]*.75));}
        const bool pass=peak<=AlbedoOutputGuard::ceiling+1e-10&&energy>1&&diff<1e-14
            &&cleanPeak>.18&&cleanPeak<.21&&stereoDiff<1e-8;
        ok &=pass;
        std::cout<<sr<<"Hz peak="<<peak<<" diff="<<diff<<" RMS="<<std::sqrt(energy/n)
                 <<" clean="<<cleanPeak<<" stereoErr="<<stereoDiff<<" pass="<<pass<<"\n";
    }
    return ok?0:1;
}
