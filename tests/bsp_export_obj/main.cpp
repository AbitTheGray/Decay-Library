#include <Decay/Bsp/BspFile.hpp>
#include <Decay/Bsp/BspTree.hpp>

int main()
{
    using namespace Decay::Bsp;

    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");

    auto tree = std::make_shared<BspTree>(bsp);
    tree->ExportFlatObj("de_dust2.obj");
    tree->ExportMtl("de_dust2.mtl", "textures");
    tree->ExportTextures("textures");
}
