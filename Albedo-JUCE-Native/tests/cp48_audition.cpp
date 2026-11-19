#include "AlbedoPrograms.hpp"
#include "AlbedoPreviousBank.hpp"
#include "vitriol_dsp.hpp"
#include "albedo_factory_patches.hpp"
#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

int main(int argc, char** argv)
{
    if(argc != 3) return 2;
    const std::filesystem::path directory(argv[2]);
    std::filesystem::create_directories(directory);
    for(int index=0;index<128;++index) {
        auto synth=std::make_unique<vitriol::Synth>();
        if(std::string(argv[1])=="old") {
            const auto& base=albedo_factory::patches[index<64 ? index : albedo_previous::patches[index-64].baseIndex];
            for(size_t i=0;i<albedo_factory::parameterIds.size();++i)
                synth->setParam(albedo_factory::parameterIds[i],base.values[i]);
            if(index>=64) {
                const auto& patch=albedo_previous::patches[index-64];
                for(size_t i=0;i<patch.changeCount;++i)
                    synth->setParam(patch.changes[i].id,patch.changes[i].value);
            }
        } else {
            albedo_programs::apply(index,[&](uint32_t id,float value){synth->setParam(id,value);});
        }
        synth->prepare(48000);
        const std::string role=index<64 ? albedo_material::patches[index].role : albedo_creative::patches[index-64].role;
        const bool held=role=="Pad" || role=="Drone" || role=="Texture";
        const bool chord=held || role=="Keys";
        const int root=role=="Bass" ? 38 : 57;
        std::ofstream stream(directory/(std::to_string(index)+".f32"),std::ios::binary);
        std::array<double,128> left{},right{};
        std::array<float,256> stereo{};
        for(int block=0;block<975;++block) {
            if(block==0 || (!held && block==210)) {
                int note=root+(block ? 7 : 0);
                synth->noteOn(note,block ? .71 : .84);
                if(chord) {synth->noteOn(note+3,.68);synth->noteOn(note+10,.61);}
            }
            if(block==(held ? 605 : 120) || (!held && block==380)) {
                int note=root+(block==380 ? 7 : 0);
                synth->noteOff(note);
                if(chord){synth->noteOff(note+3);synth->noteOff(note+10);}
            }
            synth->process64(left.data(),right.data(),128);
            for(int i=0;i<128;++i){stereo[2*i]=float(left[i]);stereo[2*i+1]=float(right[i]);}
            stream.write(reinterpret_cast<const char*>(stereo.data()),sizeof(stereo));
        }
        if(!stream) return 1;
    }
    return 0;
}
