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
    std::shared_ptr<BspFile>& ptr = BspFiles.emplace_back(std::make_shared<BspFile>(path));
    return ptr.get();
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

    std::shared_ptr<BspTree>& ptr = BspTrees.emplace_back(std::make_shared<BspTree>(bsp));
    return ptr.get();
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
