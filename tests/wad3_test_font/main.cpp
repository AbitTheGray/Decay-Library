#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    auto wad = WadFile("../../../half-life/valve/gfx.wad");

    WadFile::Font font = wad.ReadFont("CONCHARS");

    //TODO Check font character data against expected

    // Save test
    font.WriteRgbPng("CONCHARS.png");
    font.WriteRgbaPng("CONCHARS_.png");

    font.WriteCharacterPngs("chars");
}
