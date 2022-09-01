#include <Decay/Wad/WadFile.hpp>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadFile("../../../half-life/valve/fonts.wad");
    //auto wad = WadFile("../../../half-life/valve/gfx.wad");

    {
        for(const auto& item : wad.GetItems())
        {
            if(item.Type != WadFile::ItemType::Font)
                continue;

            std::fstream(item.Name + ".bin", std::ios_base::out | std::ios_base::binary).write(static_cast<const char*>(item.Data), item.Size);
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
