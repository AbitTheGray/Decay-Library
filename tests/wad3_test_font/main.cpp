#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    std::fstream in("../../../half-life/valve/gfx.wad", std::ios_base::in | std::ios_base::binary);
    R_ASSERT(in.good(), "Failed to open the file");
    auto wad = WadFile(in);

    WadFile::Font font = wad.ReadFont("CONCHARS");

    //TODO Check font character data against expected

    // Save test
    font.WriteRgbPng("CONCHARS.png");
    font.WriteRgbaPng("CONCHARS_.png");

    font.WriteCharacterPngs("chars");
}
