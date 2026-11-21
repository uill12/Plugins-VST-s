#include "AlbedoOutputGuard.hpp"
#include <deque>
#include <iostream>
int main(){
 AlbedoOutputGuard g; g.prepare(48000);
 std::deque<double> window;
 double dc=0, gain=1, hist[3]{}, err=0;
 const double pole=exp(-2*3.14159265358979323846*1.5/48000), release=exp(-1./(48000*.115));
 for(int i=0;i<1500;i++){
  double l=1.8*exp(-i*.0009),r=l; dc=pole*dc+(1-pole)*l;
  double x=l-dc, peak=std::abs(x);
  for(double t:{.25,.5,.75}){
   auto [a,b,c]=hist;
   double v=b+.5*t*(c-a+t*(2*a-5*b+4*c-x+t*(-a+3*b-3*c+x)));
   peak=std::max(peak,std::abs(v));
  }
  hist[0]=hist[1];hist[1]=hist[2];hist[2]=x;
  window.push_back(peak);if(window.size()>64)window.pop_front();
  double target=std::min(1.,.88 / *std::max_element(window.begin(),window.end()));
  gain=target<gain?target:release*gain+(1-release)*target;
  g.process(&l,&r,1);err=std::max(err,std::abs(gain-g.currentGain()));
 }
 std::cout<<"maximum gain error "<<err<<"\n";return err>1e-12;
}
