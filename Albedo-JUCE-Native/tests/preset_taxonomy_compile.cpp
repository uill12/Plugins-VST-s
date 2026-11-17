#include "AlbedoPresetTaxonomy.hpp"
#include "albedo_factory_patches.hpp"
#include <cassert>
#include <string>
int main()
{
    static_assert(albedo_preset_taxonomy::entries.size()==albedo_factory::patches.size());
    for (int i=0; i<64; ++i)
    {
        auto& c=albedo_preset_taxonomy::byIndex(i);
        assert(c.index == i);
        auto name=std::wstring(albedo_factory::patches[(size_t)i].name);
        auto expected=std::wstring(c.name, c.name+std::string(c.name).size());
        assert(name==expected);
        assert(c.family && *c.family && c.role && *c.role);
        assert(c.character && *c.character && c.motion && *c.motion && c.context && *c.context);
    }
}
