#include <Decay/Wad/WadFile.hpp>
#include <iostream>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadFile("../../../half-life/valve/gfx.wad");

    WadFile::Image image = wad.ReadImage("LAMBDA");

    //TODO Check image data against expected

    // Save test
    image.WriteRgbPng("LAMBDA.png");
    image.WriteRgbaPng("LAMBDA_.png");
}
