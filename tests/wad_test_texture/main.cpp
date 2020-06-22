#include <Decay/Wad/WadFile.hpp>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadFile("../../../half-life/valve/halflife.wad");

    WadFile::Texture texture = wad.ReadTexture("AAATRIGGER");

    //TODO Check texture data against expected

    // Save test
    texture.WriteRgbPng("AAATRIGGER.png");
    texture.WriteRgbaPng("AAATRIGGER_.png");
}
