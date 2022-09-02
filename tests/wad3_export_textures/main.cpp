#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    auto wad = WadFile("../../../half-life/cstrike/cstrike.wad");

    wad.ExportTextures("textures");
}
