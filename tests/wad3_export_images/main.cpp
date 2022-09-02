#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    auto wad = WadFile("../../../half-life/valve/gfx.wad");
    //auto wad = WadFile("../../../half-life/valve/cached.wad");

    /*
    {
        for(const auto& item : wad.GetItems())
        {
            if(item.Type != WadFile::ItemType::Image)
                continue;

            std::fstream(item.Name + ".bin", std::ios_base::out | std::ios_base::binary).write(static_cast<const char*>(item.Data), item.Size);
        }
    }
    */

    for(auto& kv : wad.ReadAllImages_Map())
    {
        const auto& name = kv.first;
        WadFile::Image& img = kv.second;

        img.WriteRgbPng(name + ".png");

        img.Palette[255] = glm::u8vec3{ 0x00, 0x00, 0xFF };
        img.WriteRgbaPng(name + "_.png");
    }
}
