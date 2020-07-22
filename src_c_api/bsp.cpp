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

bsp_file* bsp_file_load(const char* path, char* error)
{
    if(path == nullptr)
        return nullptr;

    std::filesystem::path filename(path);
    if(!std::filesystem::exists(filename)) // File not found
    {
        *error = DECAY_ERR_FILE_NOT_FOUND;
        return nullptr;
    }
    if(!std::filesystem::is_regular_file(filename)) // Path target is not a file
    {
        *error = DECAY_ERR_FILE_INVALID;
        return nullptr;
    }

    try
    {
        return BspFiles.emplace_back(std::make_shared<BspFile>(path)).get();
    }
    catch(std::exception& ex)
    {
        std::cerr << "bsp_file_load(\"" << path << "\") resulted in: " << ex.what() << std::endl;

        *error = DECAY_ERR_FILE_PARSE_FAILED;
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

bsp_tree* bsp_tree_create(bsp_file* bspFile, char* error)
{
    auto bsp = GetBspPointer(bspFile);
    if(bsp == nullptr)
    {
        *error = DECAY_ERR_ARG_NULL;
        return nullptr;
    }

    try
    {
        return reinterpret_cast<bsp_tree*>(BspTrees.emplace_back(std::make_shared<BspTree>(bsp)).get());
    }
    catch(std::exception& ex)
    {
        std::cerr << "bsp_tree_create(bspFile) resulted in: " << ex.what() << std::endl;

        *error = DECAY_ERR_BSP_TREE_CREATE_FAILED;
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

bsp_vertex* bsp_vertices(bsp_tree* bspTree, int* length)
{
    *length = bspTree->Vertices.size();

    return bspTree->Vertices.data();
}

bsp_lightmap bsp_light(const bsp_tree* bspTree)
{
    auto& light = bspTree->Light;
    return {
        light.Width,
        light.Height,
        light.Data.data()
    };
}
