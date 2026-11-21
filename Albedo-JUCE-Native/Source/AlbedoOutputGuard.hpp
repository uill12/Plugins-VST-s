#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// CP53: stereo-linked transparent true-peak-aware output protector.
// No allocations, locks, transient hard clipping or GUI activity in audio thread.
// 63-sample lookahead; Catmull-Rom is used only for peak estimation.
class AlbedoOutputGuard final
{
public:
    static constexpr int lookahead = 64;
    static constexpr int latency = lookahead - 1;
    static constexpr double ceiling = 0.88; // ~-1.11 dBFS

    void prepare(double sampleRate) noexcept
    {
        if(!std::isfinite(sampleRate) || sampleRate<8000) sampleRate=48000;
        dcPole=std::exp(-2.0*3.14159265358979323846*1.5/sampleRate);
        releasePole=std::exp(-1.0/(sampleRate*0.115));
        reset();
    }

    void reset() noexcept
    {
        delayedL.fill(0); delayedR.fill(0);
        queuePeak.fill(0); queueIndex.fill(0);
        historyL.fill(0); historyR.fill(0);
        dcL=dcR=0;
        gain=1;
        write=0;
        front=back=clock=0;
    }

    template<typename T> void process(T* left,T* right,int count) noexcept
    {
        if(left==nullptr || right==nullptr || count<=0) return;
        for(int i=0;i<count;++i)
        {
            double l=std::isfinite(left[i])?double(left[i]):0.0;
            double r=std::isfinite(right[i])?double(right[i]):0.0;
            // 1.5 Hz DC rejection does not audibly thin sub-bass.
            dcL=dcPole*dcL+(1.0-dcPole)*l;
            dcR=dcPole*dcR+(1.0-dcPole)*r;
            l-=dcL; r-=dcR;

            const double peak=std::max(estimatePeak(l,historyL),estimatePeak(r,historyR));
            // Linked sliding peak maximum; monotonic deque O(1) amortized.
            while(back>front && queuePeak[(back-1)%lookahead]<=peak) --back;
            queuePeak[back%lookahead]=peak;
            queueIndex[back%lookahead]=clock;
            ++back;
            while(back>front && queueIndex[front%lookahead]+lookahead<=clock) ++front;
            const double maximum=queuePeak[front%lookahead];
            const double target=maximum>ceiling ? ceiling/maximum : 1.0;
            gain=target<gain ? target : releasePole*gain+(1.0-releasePole)*target;

            delayedL[write]=l; delayedR[write]=r;
            write=(write+1)%lookahead;
            const double outL=delayedL[write]*gain;
            const double outR=delayedR[write]*gain;
            left[i]=static_cast<T>(std::clamp(outL,-ceiling,ceiling));
            right[i]=static_cast<T>(std::clamp(outR,-ceiling,ceiling));
            ++clock;
        }
    }

    double currentGain() const noexcept {return gain;}
private:
    static double interpolate(double p0,double p1,double p2,double p3,double t) noexcept
    {
        return p1+.5*t*(p2-p0+t*(2*p0-5*p1+4*p2-p3+t*(-p0+3*p1-3*p2+p3)));
    }
    static double estimatePeak(double input,std::array<double,3>& history) noexcept
    {
        const double p0=history[0],p1=history[1],p2=history[2];
        const double peak=std::max({std::abs(input),
            std::abs(interpolate(p0,p1,p2,input,.25)),
            std::abs(interpolate(p0,p1,p2,input,.5)),
            std::abs(interpolate(p0,p1,p2,input,.75))});
        history[0]=p1;history[1]=p2;history[2]=input;
        return peak;
    }
    std::array<double,lookahead> delayedL{},delayedR{},queuePeak{};
    std::array<uint64_t,lookahead> queueIndex{};
    std::array<double,3> historyL{},historyR{};
    int write=0;
    uint64_t front=0,back=0,clock=0;
    double dcL=0,dcR=0,dcPole=1,releasePole=1,gain=1;
};
