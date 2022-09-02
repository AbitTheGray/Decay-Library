#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    auto wad = WadFile("../../../half-life/valve/halflife.wad");

    WadFile::Texture texture = wad.ReadTexture("AAATRIGGER");

    //TODO Check texture data against expected

    // Save test
    texture.WriteRgbPng("AAATRIGGER.png");
    texture.WriteRgbaPng("AAATRIGGER_.png");
}
