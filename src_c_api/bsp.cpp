#include "bsp.h"

#include <Decay/Bsp/BspFile.hpp>
#include <Decay/Bsp/BspTree.hpp>

using namespace Decay::Bsp;

std::vector<std::shared_ptr<BspFile>> BspFiles = {};

inline static std::shared_ptr<BspFile> GetBspPointer(bsp_file* bspFile)
{
    for(auto& bsp : BspFiles)
        if(bsp.get() == bspFile)
            return bsp;
    return nullptr;
}

bsp_file* bsp_file_load(const char* path)
{
    if(path == nullptr)
        return nullptr;

    std::filesystem::path filename(path);
    if(!std::filesystem::exists(filename))
        return nullptr; // File not found
    if(!std::filesystem::is_regular_file(filename))
        return nullptr; // Path target is not a file

    try
    {
        return BspFiles.emplace_back(std::make_shared<BspFile>(path)).get();
    }
    catch(std::exception& ex)
    {
        std::cerr << "bsp_file_load(\"" << path << "\") resulted in: " << ex.what() << std::endl;
        return nullptr;
    }
}

void bsp_file_free(bsp_file* bspFile)
{
    std::size_t i;
    for(i = 0; i < BspFiles.size(); i++)
    {
        const auto& vptr = BspFiles[i];
        if(vptr.get() == bspFile)
            break;
    }

    if(i == BspFiles.size())
        return;
    BspFiles.erase(BspFiles.begin() + i);
}

std::vector<std::shared_ptr<BspTree>> BspTrees = {};

bsp_tree* bsp_tree_create(bsp_file* bspFile)
{
    auto bsp = GetBspPointer(bspFile);
    if(bsp == nullptr)
        return nullptr;

    try
    {
        return reinterpret_cast<bsp_tree*>(BspTrees.emplace_back(std::make_shared<BspTree>(bsp)).get());
    }
    catch(std::exception& ex)
    {
        std::cerr << "bsp_tree_create(bspFile) resulted in: " << ex.what() << std::endl;
        return nullptr;
    }
}

void bsp_tree_free(bsp_tree* bspTree)
{
    std::size_t i;
    for(i = 0; i < BspTrees.size(); i++)
    {
        const auto& vptr = BspTrees[i];
        if(vptr.get() == bspTree)
            break;
    }

    if(i == BspTrees.size())
        return;
    BspTrees.erase(BspTrees.begin() + i);
}

bsp_vertex* bsp_vertices(bsp_tree* bspTree)
{
    return bspTree->Vertices.data();
}

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

int bsp_model_get_indices(bsp_model* model, int textureIndex, int* indices)
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
