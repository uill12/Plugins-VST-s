#pragma once
#include "AlbedoCreativeBank.hpp"
#include "AlbedoMaterialBank.hpp"
#include "AlbedoMorphBank.hpp"
#include "AlbedoProgramTrim.hpp"
namespace albedo_programs {
template<typename Set>
void apply(int index, Set&& set)
{
    auto load = [&](const auto& p) {
        set(6031,0.f);
        set(6032,0.f);
        bool hasArp=false,hasExplicitClock=false;
        for (std::size_t i=0; i<p.changeCount; ++i) {
            const auto& c=p.changes[i];
            if(c.id==6020)hasArp=c.value>0.5f;
            if(c.id==6030)hasExplicitClock=true;
            set(c.id, c.value + (c.id==9000 ? albedo_program_trim[index] : 0.0f));
        }
        // Only rhythmic factory patches opt in; an explicit FREE clock wins.
        if(hasArp&&!hasExplicitClock)set(6030,1.0f);
    };
    if(index<64) load(albedo_material::patches[index]);
    else if(index<128) load(albedo_creative::patches[index-64]);
    else if(index<192) load(albedo_morph::patches[index-128]);
}
}
