#include "AlbedoPrograms.hpp"

#include "vitriol_dsp.hpp"
#include "albedo_factory_patches.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

int main(int argc, char** argv)
{
    if(argc != 2) return 2;
    const std::filesystem::path directory(argv[1]);
    std::filesystem::create_directories(directory);
    for(int index=0;index<128;++index) {
        auto synth=std::make_unique<vitriol::Synth>();
        albedo_programs::apply(index,[&](uint32_t id,float value){synth->setParam(id,value);});
        synth->prepare(48000);
        std::ofstream stream(directory/(std::to_string(index)+".f32"),std::ios::binary);
        std::array<double,128> left{},right{};
        std::array<float,256> stereo{};
        // Identical low and high notes across every role, including release tails.
        for(int block=0;block<900;++block) {
            if(block==0)synth->noteOn(48,.8);
            if(block==260)synth->noteOff(48);
            if(block==450)synth->noteOn(67,.8);
            if(block==710)synth->noteOff(67);
            synth->process64(left.data(),right.data(),128);
            for(int i=0;i<128;++i){stereo[2*i]=float(left[i]);stereo[2*i+1]=float(right[i]);}
            stream.write(reinterpret_cast<const char*>(stereo.data()),sizeof(stereo));
        }
        if(!stream) return 1;
    }
    return 0;
}
