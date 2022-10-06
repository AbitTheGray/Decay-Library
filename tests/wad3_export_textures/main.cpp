#include "Decay/Wad/Wad3/WadFile.hpp"

int main()
{
    using namespace Decay::Wad::Wad3;

    std::fstream in("../../../half-life/cstrike/cstrike.wad", std::ios_base::in | std::ios_base::binary);
    R_ASSERT(in.good(), "Failed to open the file");
    auto wad = WadFile(in);

    wad.ExportTextures("textures");
}
