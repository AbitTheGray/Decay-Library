#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    std::fstream in("../../../half-life/valve/gfx.wad", std::ios_base::in | std::ios_base::binary);
    R_ASSERT(in.good(), "Failed to open the file");
    auto wad = WadFile(in);

    WadFile::Image image = wad.ReadImage("LAMBDA");

    //TODO Check image data against expected

    // Save test
    image.WriteRgbPng("LAMBDA.png");
    image.WriteRgbaPng("LAMBDA_.png");
}
