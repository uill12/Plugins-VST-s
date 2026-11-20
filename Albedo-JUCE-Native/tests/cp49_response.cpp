#include "vitriol_dsp.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

static std::vector<double> probe(int method,int id,double value)
{
    auto s=std::make_unique<vitriol::Synth>();
    s->setParam(1000,method);
    s->setParam(2100,18); s->setParam(2101,24);
    s->setParam(1001,1); s->setParam(1002,92); s->setParam(1006,62);
    s->setParam(1007,44); s->setParam(1008,58); s->setParam(1009,71);
    s->setParam(1011,3); s->setParam(1012,18); s->setParam(1013,80);
    s->setParam(1032,1); s->setParam(1064,6); s->setParam(1096,8);
    s->setParam(1033,1); s->setParam(1065,1); s->setParam(1097,1);
    s->setParam(1034,76); s->setParam(1066,73); s->setParam(1098,80);
    s->setParam(2200,80); s->setParam(4000,8); s->setParam(4001,85);
    s->setParam(5004,55); s->setParam(5005,1); s->setParam(3102,68);
    s->setParam(3100,.009); s->setParam(3103,.19); s->setParam(9002,1);
    s->setParam(id,value); s->prepare(48000);
    std::vector<double> out(2*24000),r(128),l(128);
    s->noteOn(53,.63);
    for(int i=0;i<24000;i+=128){
        if(i==15360)s->noteOff(53);
        int n=std::min(128,24000-i);s->process64(l.data(),r.data(),n);
        for(int j=0;j<n;j++){out[2*(i+j)]=l[j];out[2*(i+j)+1]=r[j];}
    }
    return out;
}
int main(){
    bool ok=true;
    auto test=[&](int m,int id,double lo,double hi){
        auto a=probe(m,id,lo),b=probe(m,id,hi);double delta=0,power=0;
        for(size_t i=0;i<a.size();i++){delta+=std::pow(a[i]-b[i],2);power+=a[i]*a[i]+b[i]*b[i];}
        double ratio=std::sqrt(delta/std::max(power,1e-24));
        std::cout<<m<<" "<<id<<" "<<ratio<<"\n";ok &=std::isfinite(ratio)&&ratio>.00001;
    };
    for(int m=0;m<10;m++)for(int id=1006;id<=1009;id++)test(m,id,12,88);
    for(int id: {1012,1013,2000,2001,2002,2003,2004,2005,2006,2100,2101,2200,3001,3102,4001})
        test(0,id,2,id==1012?37:98);
    test(0,3000,170,12000);test(0,3100,.003,.3);test(0,3103,.02,.6);
    return ok?0:1;
}
