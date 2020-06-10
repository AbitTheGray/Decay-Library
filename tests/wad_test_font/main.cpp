#include <Decay/Wad/WadParser.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadParser("../../../half-life/valve/gfx.wad");

    WadParser::Font font = wad.ReadFont("CONCHARS");

    //TODO Check font character data against expected

    // Save test
    font.WriteRgbPng("CONCHARS.png");
    font.WriteRgbaPng("CONCHARS_.png");
}
