#include <Decay/Wad/WadFile.hpp>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadFile("../../../half-life/valve/gfx.wad");

    WadFile::Font font = wad.ReadFont("CONCHARS");

    //TODO Check font character data against expected

    // Save test
    font.WriteRgbPng("CONCHARS.png");
    font.WriteRgbaPng("CONCHARS_.png");

    font.WriteCharacterPngs("chars");
}
