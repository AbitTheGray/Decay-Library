#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    std::fstream in("../../../half-life/valve/fonts.wad", std::ios_base::in | std::ios_base::binary);
    //std::fstream in("../../../half-life/valve/gfx.wad", std::ios_base::in | std::ios_base::binary);
    R_ASSERT(in.good(), "Failed to open the file");
    auto wad = WadFile(in);

    {
        for(const auto& item : wad.GetItems())
        {
            if(item.Type != WadFile::ItemType::Font)
                continue;

            std::fstream itemOut(item.Name + ".bin", std::ios_base::out | std::ios_base::binary);
            itemOut.write(reinterpret_cast<const char*>(item.Data.data()), item.Data.size());
        }
    }

    for(auto& kv : wad.ReadAllFonts_Map())
    {
        const auto& name = kv.first;
        WadFile::Font& font = kv.second;

        font.WriteRgbPng(name + ".png");

        font.Palette[255] = glm::u8vec3{ 0x00, 0x00, 0xFF };
        font.WriteRgbaPng(name + "_.png");

        font.WriteCharacterPngs(name);
    }
}
