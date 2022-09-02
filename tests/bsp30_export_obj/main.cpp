#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

int main()
{
    using namespace Decay::Bsp::v30;

    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");

    auto tree = std::make_shared<BspTree>(bsp);
    tree->ExportFlatObj("de_dust2.obj");
    tree->ExportMtl("de_dust2.mtl", "textures");
    tree->ExportTextures("textures");
}
