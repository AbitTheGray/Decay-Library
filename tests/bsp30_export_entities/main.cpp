#include <iostream>

#include "Decay/Bsp/v30/BspFile.hpp"
#include "Decay/Bsp/v30/BspTree.hpp"

int main()
{
    using namespace Decay::Bsp::v30;

    auto bsp = std::make_shared<BspFile>("../../../half-life/cstrike/maps/de_dust2.bsp");
    auto entities = BspEntities(*bsp);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    entities.ExportJson("entities_raw.json");
#pragma clang diagnostic pop

#ifdef DECAY_JSON_LIB

    {
        nlohmann::json jEntities = entities.AsJson();
        std::fstream out = std::fstream("entities.json", std::ios_base::out);
        out << jEntities.dump(2);
    }

#else
    tree.ExportEntitiesJson("entities.json");
#endif
}
