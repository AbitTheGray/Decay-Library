#include <iostream>

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

int main()
{
    using namespace Decay::Bsp::v30;

    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");
    auto tree = BspTree(bsp);

    tree.ExportEntitiesJson("entities.json");
}
