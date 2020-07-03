#include "bsp.h"

#include <Decay/Bsp/BspTree.hpp>

using namespace Decay::Bsp;

int bsp_tree_entities(bsp_tree* bspTree, const bsp_entity** entities)
{
    if(entities != nullptr)
    {
        for(std::size_t i = 0; i < bspTree->Entities.size(); i++)
            entities[i] = bspTree->Entities.data() + i;
    }

    return bspTree->Entities.size();
}

int bsp_entity_keys(bsp_entity* entity, const char** keys)
{
    if(keys != nullptr)
    {
        std::size_t i = 0;
        for(const auto& it : *entity)
            keys[i++] = it.first.c_str();
    }

    return entity->size();
}

const char* bsp_entity_value(bsp_entity* entity, const char* key)
{
    auto it = entity->find(key);
    if(it == entity->end())
        return nullptr;
    else
        return it->second.c_str();
}
