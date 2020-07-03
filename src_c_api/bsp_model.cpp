#include "bsp.h"

#include <Decay/Bsp/BspTree.hpp>

using namespace Decay::Bsp;

int bsp_get_models(bsp_tree* bspTree, bsp_model** models)
{
    if(models != nullptr)
    {
        for(std::size_t i = 0; i < bspTree->Models.size(); i++)
            models[i] = bspTree->Models[i].get();
    }

    return bspTree->Models.size();
}

bsp_model* bsp_get_model(bsp_tree* bspTree, int modelId)
{
    return bspTree->Models[modelId].get();
}

int bsp_model_textures(bsp_model* model, int* textures)
{
    if(textures != nullptr)
    {
        std::size_t i = 0;
        for(auto& it : model->Indices)
            textures[i++] = it.first;
    }

    return model->Indices.size();
}

int bsp_model_get_indices(bsp_model* model, int textureIndex, short* indices)
{
    auto& ind = model->Indices[textureIndex];

    if(indices != nullptr)
    {
        std::size_t i = 0;
        for(auto& index : ind)
            indices[i++] = index;
    }

    return ind.size();
}

bsp_vec3 bsp_model_origin(bsp_model* model)
{
    return model->Origin;
}

bsp_bounding_box bsp_model_bounding_box(bsp_model* model)
{
    return { model->BB_Min, model->BB_Max };
}
