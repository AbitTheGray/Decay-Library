#include <Decay/Wad/WadParser.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadParser("../../../half-life/valve/halflife.wad");

    WadParser::Texture texture = wad.ReadTexture("AAATRIGGER");

    //TODO Check texture data against expected

    // Save test
    texture.WriteRgbPng("AAATRIGGER.png");
    texture.WriteRgbaPng("AAATRIGGER_.png");
}
