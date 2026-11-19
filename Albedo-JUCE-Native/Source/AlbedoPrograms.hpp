#pragma once
#include "AlbedoCreativeBank.hpp"
#include "AlbedoMaterialBank.hpp"
#include "AlbedoProgramTrim.hpp"
namespace albedo_programs {
template<typename Set>
void apply(int index, Set&& set)
{
    auto load = [&](const auto& p) {
        for (std::size_t i=0; i<p.changeCount; ++i) {
            const auto& c=p.changes[i];
            set(c.id, c.value + (c.id==9000 ? albedo_program_trim[index] : 0.0f));
        }
    };
    if(index<64) load(albedo_material::patches[index]);
    else load(albedo_creative::patches[index-64]);
}
}
