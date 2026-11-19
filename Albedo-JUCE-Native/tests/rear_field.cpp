#include "../Source/AlbedoRearField.hpp"
#include <iostream>
#include <cstdlib>
static void require(bool ok) { if (!ok) std::exit(1); }
int main() {
 for(int rate : {44100,48000,88200,96000,192000}){
  const int n=rate;
  std::vector<double> in(n),l(n),r(n),a(n),b(n); in[0]=1;
  AlbedoRearField full,chunk;full.prepare(rate);chunk.prepare(rate);
  full.render(in.data(),in.data(),l.data(),r.data(),n);
  for(int i=0;i<n;i+=127)chunk.render(in.data()+i,in.data()+i,a.data()+i,b.data()+i,std::min(127,n-i));
  require(l==a && r==b);
  double e=0,f=0,c=0;for(int i=0;i<n;++i){e+=l[i]*l[i];f+=r[i]*r[i];c+=l[i]*r[i];}
  require(e>.001&&f>.001&&std::abs(c/std::sqrt(e*f))<.7);
  std::fill(in.begin(),in.end(),1.0);full.prepare(rate);full.render(in.data(),in.data(),l.data(),r.data(),n);
  require(std::abs(l.back())<1e-9 && std::abs(r.back())<1e-9);
  std::cout<<rate<<" Hz: impulse correlation "<<c/std::sqrt(e*f)<<"\n";
 }
}
