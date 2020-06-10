#include <Decay/Wad/WadParser.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadParser("../../../half-life/valve/gfx.wad");

    WadParser::Image image = wad.ReadImage("LAMBDA");

    //TODO Check image data against expected

    // Save test
    image.WriteRgbPng("LAMBDA.png");
    image.WriteRgbaPng("LAMBDA_.png");
}
