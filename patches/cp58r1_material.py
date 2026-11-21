"""Apply after CP58's DSP reconstruction."""
from pathlib import Path
import sys
path = Path(sys.argv[1]) / "src/vitriol_dsp.cpp"
source = path.read_text()
def replace_once(old, new):
    global source
    assert source.count(old) == 1, old
    source = source.replace(old, new)

replace_once(""" auto read=[&](const std::vector<float>& data){
  auto at=[&](int j){return double(data[(j+int(data.size()))%int(data.size())]);};
  return cubic(at(i-1),at(i),at(i+1),at(i+2),t);
 };""", """ const int frames=int(v.size());
 const int prev=i==0?frames-1:i-1;
 const int next=i+1==frames?0:i+1;
 const int after=next+1==frames?0:next+1;
 auto read=[&](const std::vector<float>& data){
  return cubic(data[prev],data[i],data[next],data[after],t);
 };""")
replace_once("const double material=bankReady_?materialSample(int(get(1000+e*32+10)),phase,v.sampleSide[e]):0.;",
"""double material=0.;
  if(bankReady_){
   // Recorded material has its own playback cursor, pitched from middle C.
   // Oscillator phase would scan the entire recording once per audio cycle.
   auto& cursor=v.samplePos[e];
   material=materialSample(int(get(1000+e*32+10)),cursor,v.sampleSide[e]);
   cursor=fract(cursor+(freq/261.625565)*materialSampleRate_/(fsInt*materialFrames_));
  }""")
path.write_text(source)
