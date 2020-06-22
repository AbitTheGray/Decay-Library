#include <Decay/Wad/WadFile.hpp>

int main()
{
    using namespace Decay::Wad;

    auto wad = WadFile("../../../half-life/cstrike/cstrike.wad");

    wad.ExportTextures("textures");
}
