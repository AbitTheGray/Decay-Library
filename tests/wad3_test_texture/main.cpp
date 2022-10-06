#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    std::fstream in("../../../half-life/valve/halflife.wad", std::ios_base::in | std::ios_base::binary);
    R_ASSERT(in.good(), "Failed to open the file");
    auto wad = WadFile(in);

    WadFile::Texture texture = wad.ReadTexture("AAATRIGGER");

    //TODO Check texture data against expected

    // Save test
    texture.WriteRgbPng("AAATRIGGER.png");
    texture.WriteRgbaPng("AAATRIGGER_.png");
}
