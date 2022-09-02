#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    auto wad = WadFile("../../../half-life/valve/gfx.wad");

    WadFile::Image image = wad.ReadImage("LAMBDA");

    //TODO Check image data against expected

    // Save test
    image.WriteRgbPng("LAMBDA.png");
    image.WriteRgbaPng("LAMBDA_.png");
}
