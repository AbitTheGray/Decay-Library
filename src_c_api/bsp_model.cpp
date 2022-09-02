#include "bsp.h"

#include "Decay/Bsp/v30/BspTree.hpp"

using namespace Decay::Bsp;

int bsp_get_models(bsp30_tree* bspTree, bsp30_model** models)
{
    if(models != nullptr)
    {
        for(std::size_t i = 0; i < bspTree->Models.size(); i++)
            models[i] = bspTree->Models[i].get();
    }

    return bspTree->Models.size();
}

bsp30_model* bsp_get_model(bsp30_tree* bspTree, int modelId)
{
    return bspTree->Models[modelId].get();
}

int bsp_model_textures(bsp30_model* model, int* textures)
{
    if(textures != nullptr)
    {
        std::size_t i = 0;
        for(auto& it : model->Indices)
            textures[i++] = it.first;
    }

    return model->Indices.size();
}

int bsp_model_get_indices(bsp30_model* model, int textureIndex, short* indices)
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

bsp_vec3 bsp_model_origin(bsp30_model* model)
{
    return model->Origin;
}

bsp_bounding_box bsp_model_bounding_box(bsp30_model* model)
{
    return { model->BB_Min, model->BB_Max };
}
